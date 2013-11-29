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
    mSettings(new QSettings(this)),
    mUndoGroup(new QUndoGroup(this))
{
    ui->setupUi(this);
    ui->statusBar->hide();

    QAction *undoAction = mUndoGroup->createUndoAction(this, tr("Undo"));
    QAction *redoAction = mUndoGroup->createRedoAction(this, tr("Redo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));
    undoIcon.addFile(QLatin1String(":images/24x24/edit-undo.png"));
    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    redoIcon.addFile(QLatin1String(":images/24x24/edit-redo.png"));
    undoAction->setIcon(undoIcon);
    redoAction->setIcon(redoIcon);
//    Tiled::Utils::setThemeIcon(undoAction, "edit-undo");
//    Tiled::Utils::setThemeIcon(redoAction, "edit-redo");
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), undoAction);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(1), redoAction);
    ui->menuEdit->insertSeparator(ui->menuEdit->actions().at(2));

    new ProjectActions(ui);

    new ToolManager;

    connect(docman(), SIGNAL(documentAdded(Document*)),
            SLOT(documentAdded(Document*)));
    connect(docman(), SIGNAL(currentDocumentChanged(Document*)),
            SLOT(currentDocumentChanged(Document*)));

    // Do this after connect() calls above -> esp. documentAdded()
    mWelcomeMode = new WelcomeMode(this);
    mEditMode = new EditMode(this);

    ::Utils::StyleHelper::setBaseColor(::Utils::StyleHelper::DEFAULT_BASE_COLOR);
    mTabWidget = new Core::Internal::FancyTabWidget;
    mTabWidget->setObjectName(QLatin1String("FancyTabWidget"));
    mTabWidget->statusBar()->setVisible(false);
    new ModeManager(mTabWidget, this);
    ModeManager::instance()->addMode(mWelcomeMode);
    ModeManager::instance()->addMode(mEditMode);
    setCentralWidget(mTabWidget);

    mWelcomeMode->setEnabled(true);
    ModeManager::instance()->setCurrentMode(mWelcomeMode);
    connect(ModeManager::instance(), SIGNAL(currentModeAboutToChange(IMode*)),
            SLOT(currentModeAboutToChange(IMode*)));
    connect(ModeManager::instance(), SIGNAL(currentModeChanged()),
            SLOT(currentModeChanged()));
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

    if (ModeManager::instance()->currentMode() == mWelcomeMode)
        ModeManager::instance()->setCurrentMode(mEditMode);

    if (isMinimized())
        setWindowState(windowState() & (~Qt::WindowMinimized | Qt::WindowActive));

    int ret = QMessageBox::warning(
            this, tr("Unsaved Changes"),
            tr("There are unsaved changes. Do you want to save now?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:    return ProjectActions::instance()->saveProject();
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

void MainWindow::writeSettings()
{

}

void MainWindow::documentAdded(Document *doc)
{
    mUndoGroup->addStack(doc->undoStack());

    mDocumentStuff[doc] = new PerDocumentStuff(doc);

    mEditMode->setEnabled(true);
}

void MainWindow::currentDocumentChanged(Document *doc)
{
    mDocumentChanging = true;

    if (mCurrentDocumentStuff) {
        if (!mWelcomeMode->isActive())
            mCurrentDocumentStuff->rememberTool();
        mCurrentDocumentStuff->document()->disconnect(this);
    }

    mCurrentDocumentStuff = doc ? mDocumentStuff[doc] : 0; // FIXME: unset when deleted

    if (mCurrentDocumentStuff) {
        ModeManager::instance()->setCurrentMode(mEditMode); // handles new documents
        mUndoGroup->setActiveStack(mCurrentDocumentStuff->document()->undoStack());
    } else {
        ToolManager::instance()->clearDocument();
        mEditMode->setEnabled(false);
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
