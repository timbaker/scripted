/*
 * Copyright 2013, Tim Baker <treectrl@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "abstracttool.h"
#include "documentmanager.h"
#include "editmode.h"
#include "fancytabwidget.h"
#include "luamode.h"
#include "projectactions.h"
#include "projectdocument.h"
#include "toolmanager.h"
#include "welcomemode.h"
#include "utils/stylehelper.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QUndoGroup>

SINGLETON_IMPL(MainWindow)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mCurrentDocumentStuff(0),
    mUndoGroup(new QUndoGroup(this))
{
    ui->setupUi(this);
    ui->statusBar->hide();

    new ProjectActions(ui);

    new ToolManager;

    connect(docman(), SIGNAL(documentAdded(Document*)),
            SLOT(documentAdded(Document*)));
    connect(docman(), SIGNAL(currentDocumentChanged(Document*)),
            SLOT(currentDocumentChanged(Document*)));

    // Do this after connect() calls above -> esp. documentAdded()
    mWelcomeMode = new WelcomeMode(this);
    mEditMode = new EditMode(this);
    mLuaMode = new LuaMode(this);

    ::Utils::StyleHelper::setBaseColor(::Utils::StyleHelper::DEFAULT_BASE_COLOR);
    mTabWidget = new Core::Internal::FancyTabWidget;
    mTabWidget->setObjectName(QLatin1String("FancyTabWidget"));
    mTabWidget->statusBar()->setVisible(false);
    new ModeManager(mTabWidget, this);
    ModeManager::instance()->addMode(mWelcomeMode);
    ModeManager::instance()->addMode(mEditMode);
    ModeManager::instance()->addMode(mLuaMode);
    setCentralWidget(mTabWidget);

    mWelcomeMode->setEnabled(true);
    ModeManager::instance()->setCurrentMode(mWelcomeMode);
    connect(ModeManager::instance(), SIGNAL(currentModeAboutToChange(IMode*)),
            SLOT(currentModeAboutToChange(IMode*)));
    connect(ModeManager::instance(), SIGNAL(currentModeChanged()),
            SLOT(currentModeChanged()));

    ProjectActions::instance()->updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmAllSave()) {
        writeSettings();
        docman()->closeAllDocuments();
        event->accept();
    } else
        event->ignore();

}

bool MainWindow::confirmAllSave()
{
    foreach (Document *doc, docman()->documents()) {
        if (!doc->isModified())
            continue;
        docman()->setCurrentDocument(doc);
        if (!confirmSave())
            return false;
    }
    return true;
}

bool MainWindow::confirmSave()
{
    if (!mCurrentDocumentStuff || !mCurrentDocumentStuff->document()->isModified())
        return true;

    if (mCurrentDocumentStuff->document()->isLuaDocument())
        switchToLuaMode();

    if (mCurrentDocumentStuff->document()->isProjectDocument())
        switchToEditMode();

    if (isMinimized())
        setWindowState(windowState() & (~Qt::WindowMinimized | Qt::WindowActive));

    int ret = QMessageBox::warning(
            this, tr("Unsaved Changes"),
            tr("There are unsaved changes. Do you want to save now?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:    return ProjectActions::instance()->saveFile();
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

void MainWindow::writeSettings()
{
    mSettings.beginGroup(QLatin1String("mainwindow"));
    mSettings.setValue(QLatin1String("geometry"), saveGeometry());
    mSettings.setValue(QLatin1String("state"), saveState());
    mSettings.endGroup();

    mSettings.beginGroup(QLatin1String("recentFiles"));
    if (Document *document = docman()->currentDocument())
        mSettings.setValue(QLatin1String("lastActive"), document->fileName());

    QStringList fileList;
    for (int i = 0; i < docman()->documentCount(); i++) {
        Document *doc = docman()->documentAt(i);
        if (doc->fileName().isEmpty())
            continue;
        fileList.append(doc->fileName());
    }
    mSettings.setValue(QLatin1String("lastOpenFiles"), fileList);

    mSettings.endGroup();

    mEditMode->writeSettings(mSettings);
    mLuaMode->writeSettings(mSettings);
}

void MainWindow::readSettings()
{
    mSettings.beginGroup(QLatin1String("mainwindow"));
    QByteArray geom = mSettings.value(QLatin1String("geometry")).toByteArray();
    if (!geom.isEmpty())
        restoreGeometry(geom);
    else
        resize(1024, 768);
    restoreState(mSettings.value(QLatin1String("state"), QByteArray()).toByteArray());
    mSettings.endGroup();

    mEditMode->readSettings(mSettings);
    mLuaMode->writeSettings(mSettings);
}

void MainWindow::openLastFiles()
{
    mSettings.beginGroup(QLatin1String("recentFiles"));

    QStringList lastOpenFiles = mSettings.value(
                QLatin1String("lastOpenFiles")).toStringList();

    for (int i = 0; i < lastOpenFiles.size(); i++) {
        if (ProjectActions::instance()->openFile(lastOpenFiles.at(i))) {
            // TODO: restore scale & scroll position
        }
    }

    QString lastActiveDocument = mSettings.value(QLatin1String("lastActive")).toString();
    int documentIndex = docman()->findDocument(lastActiveDocument);
    if (documentIndex != -1)
        docman()->setCurrentDocument(documentIndex);

    mSettings.endGroup();
}

void MainWindow::switchToEditMode()
{
    ModeManager::instance()->setCurrentMode(mEditMode);
}

void MainWindow::switchToLuaMode()
{
    ModeManager::instance()->setCurrentMode(mLuaMode);
}

void MainWindow::documentAdded(Document *doc)
{
    mUndoGroup->addStack(doc->undoStack());

    mDocumentStuff[doc] = new PerDocumentStuff(doc);

    mEditMode->setEnabled(docman()->projectDocuments().size());
    mLuaMode->setEnabled(docman()->luaDocuments().size());
}

void MainWindow::currentDocumentChanged(Document *doc)
{
    mDocumentChanging = true;

    if (mCurrentDocumentStuff) {
        if (!mWelcomeMode->isActive())
            mCurrentDocumentStuff->rememberTool();
        mCurrentDocumentStuff->document()->disconnect(this);
        mCurrentDocumentStuff->document()->disconnect(ProjectActions::instance());
    }

    mCurrentDocumentStuff = doc ? mDocumentStuff[doc] : 0; // FIXME: unset when deleted

    if (mCurrentDocumentStuff) {
        if (doc->isProjectDocument())
            switchToEditMode(); // handles new documents
        if (doc->isLuaDocument())
            switchToLuaMode(); // handles new documents
        mUndoGroup->setActiveStack(mCurrentDocumentStuff->document()->undoStack());
        connect(doc, SIGNAL(cleanChanged()), ProjectActions::instance(), SLOT(updateActions()));
        connect(doc, SIGNAL(fileNameChanged()), ProjectActions::instance(), SLOT(updateActions()));
    } else {
        ToolManager::instance()->clearDocument();
        mEditMode->setEnabled(false);
        mLuaMode->setEnabled(false);
    }

    ProjectActions::instance()->updateActions();
//    updateWindowTitle();

    if (mCurrentDocumentStuff && !mWelcomeMode->isActive())
        mCurrentDocumentStuff->restoreTool();

    mDocumentChanging = false;
}

void MainWindow::documentCloseRequested(int index)
{
    Document *doc = docman()->documentAt(index);
    if (doc->isModified()) {
        docman()->setCurrentDocument(index);
        if (!confirmSave())
            return;
    }
    docman()->closeDocument(index);
}

void MainWindow::currentModeAboutToChange(IMode *mode)
{

}

void MainWindow::currentModeChanged()
{

}

/////

PerDocumentStuff::PerDocumentStuff(Document *doc) :
    QObject(doc),
    mDocument(doc),
    mPrevTool(0)
{
}

void PerDocumentStuff::rememberTool()
{
    mPrevTool = ToolManager::instance()->currentTool();
}

void PerDocumentStuff::restoreTool()
{
    if (mPrevTool && mPrevTool->isEnabled())
        mPrevTool->makeCurrent();
}
