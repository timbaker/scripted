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

#include "editmode.h"

#include "basegraphicsview.h"
#include "documentmanager.h"
#include "embeddedmainwindow.h"
#include "luadockwidget.h"
#include "luadocument.h"
#include "luaeditor.h"
#include "metaeventdock.h"
#include "mainwindow.h"
#include "projectactions.h"
#include "projectdocument.h"
#include "scriptscene.h"
#include "scriptview.h"
#include "projecttreedock.h"
#include "scriptsdock.h"
#include "scriptvariablesdock.h"
#include "toolmanager.h"

#include <QDir>
#include <QTabWidget>
#include <QUndoStack>
#include <QVBoxLayout>

EditModeToolBar::EditModeToolBar(QWidget *parent) :
    QToolBar(parent)
{
    setWindowTitle(tr("Editing ToolBar"));
    setObjectName("EditMode.ToolBar");

    Ui::MainWindow *actions = ProjectActions::instance()->actions();
    addAction(actions->actionNewProject);
    addAction(actions->actionOpenProject);
    addAction(actions->actionSaveProject);
    addAction(actions->actionSaveProjectAs);
    addSeparator();
    addAction(ProjectActions::instance()->undoAction());
    addAction(ProjectActions::instance()->redoAction());
#if 0
    addAction(actions->actionSelectMoveNode);
    addAction(actions->actionAreaSelectNodes);
    addAction(actions->actionAddRemoveNode);
    addAction(actions->actionSelectMovePath);
    addAction(actions->actionCreatePath);
    addAction(actions->actionAddPathSegments);

    addSeparator();

    addAction(actions->actionPencil);
    addAction(actions->actionEraser);

    addSeparator();

    addAction(actions->actionSelectMoveImages);
#endif
}

/////

EditModePerDocumentStuff::EditModePerDocumentStuff(EditMode *mode, ProjectDocument *doc) :
    QObject(doc),
    mMode(mode),
    mDocument(doc),
    mScene(new ProjectScene(doc)),
    mView(new ProjectView(doc))
{
    mView->setScene(mScene);
    mScene->setParent(mView);

    connect(document(), SIGNAL(fileNameChanged()), SLOT(updateDocumentTab()));
    connect(document(), SIGNAL(cleanChanged()), SLOT(updateDocumentTab()));
}

EditModePerDocumentStuff::~EditModePerDocumentStuff()
{
    // widget() is added to a QTabWidget.
    // Removing a tab does not delete the page widget.
    // mScene is a child of the view.
    delete widget();
}

QWidget *EditModePerDocumentStuff::widget() const
{
    return mView;
}

void EditModePerDocumentStuff::activate()
{
    ToolManager::instance()->setScene(mScene);
}

void EditModePerDocumentStuff::deactivate()
{
//    ToolManager::instance()->setScene(0);
}

void EditModePerDocumentStuff::updateDocumentTab()
{
    int tabIndex = docman()->projectDocuments().indexOf(document());
    if (tabIndex == -1)
        return;

    QString tabText = document()->displayName();
    if (document()->isModified())
        tabText.prepend(QLatin1Char('*'));
    mMode->mTabWidget->setTabText(tabIndex, tabText);

    QString tooltipText = QDir::toNativeSeparators(document()->fileName());
    mMode->mTabWidget->setTabToolTip(tabIndex, tooltipText);
}

/////

EditMode::EditMode(QObject *parent) :
    IMode(parent),
    mProjectDock(new ProjectTreeDock),
    mLuaDock(new LuaDockWidget),
    mEventsDock(new MetaEventDock),
    mScriptsDock(new ScriptsDock),
    mVariablesDock(new ScriptVariablesDock),
    mToolBar(new EditModeToolBar),
    mCurrentDocumentStuff(0)
{
    setDisplayName(tr("Edit"));
//    setIcon(QIcon(QLatin1String(":images/24x24/document-new.png")));

    mMainWindow = new EmbeddedMainWindow;
    mMainWindow->setObjectName(QLatin1String("EditMode.Widget"));

    mTabWidget = new QTabWidget;
    mTabWidget->setObjectName(QLatin1String("EditMode.TabWidget"));
    mTabWidget->setDocumentMode(true);
    mTabWidget->setTabsClosable(true);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setObjectName(QLatin1String("EditMode.VBox"));
    vbox->setMargin(0);
    vbox->addWidget(mTabWidget);
//    vbox->addLayout(mStatusBar->statusBarLayout);
    vbox->setStretchFactor(mTabWidget, 1);

    QWidget *w = new QWidget;
    w->setObjectName(QLatin1String("EditMode.VBoxWidget"));
    w->setLayout(vbox);

    mMainWindow->setCentralWidget(w);
    mMainWindow->addToolBar(mToolBar);

    mMainWindow->registerDockWidget(mProjectDock);
    mMainWindow->registerDockWidget(mLuaDock);
    mMainWindow->registerDockWidget(mEventsDock);
    mMainWindow->registerDockWidget(mScriptsDock);
    mMainWindow->registerDockWidget(mVariablesDock);
    mMainWindow->addDockWidget(Qt::TopDockWidgetArea, mVariablesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mProjectDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mEventsDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mLuaDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mScriptsDock);
//    mMainWindow->tabifyDockWidget(mLayersDock, mFileSystemDock);

    setWidget(mMainWindow);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(currentDocumentTabChanged(int)));
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)),
            SLOT(documentTabCloseRequested(int)));

    connect(docman(), SIGNAL(documentAdded(Document*)),
            SLOT(documentAdded(Document*)));
    connect(docman(), SIGNAL(currentDocumentChanged(Document*)),
            SLOT(currentDocumentChanged(Document*)));
    connect(docman(), SIGNAL(documentAboutToClose(int,Document*)),
            SLOT(documentAboutToClose(int,Document*)));

    connect(this, SIGNAL(activeStateChanged(bool)), SLOT(onActiveStateChanged(bool)));
}

void EditMode::readSettings(QSettings &settings)
{
    settings.beginGroup(QLatin1String("EditMode"));
    mMainWindow->readSettings(settings);
    settings.endGroup();
}

void EditMode::writeSettings(QSettings &settings)
{
    settings.beginGroup(QLatin1String("EditMode"));
    mMainWindow->writeSettings(settings);
    settings.endGroup();
}

void EditMode::onActiveStateChanged(bool active)
{
#if 0
    QMenu *menu = MainWindow::instance()->actionsIface()->menuViews;
    menu->clear();
#endif
    if (active) {
        if (mCurrentDocumentStuff)
            mCurrentDocumentStuff->activate();

        // When switching from one mode to another, set the current document
        currentDocumentTabChanged(mTabWidget->currentIndex());
#if 0
        QMap<QString,QAction*> map;
        foreach (QDockWidget *dockWidget, mMainWindow->dockWidgets()) {
            QAction *action = dockWidget->toggleViewAction();
            map[action->text()] = action;
        }
        foreach (QAction *action, map.values())
            menu->addAction(action);
        menu->addSeparator();
        foreach (QToolBar *toolBar, mMainWindow->toolBars()) {
            menu->addAction(toolBar->toggleViewAction());
        }
#endif
    } else {
        if (mCurrentDocumentStuff)
            mCurrentDocumentStuff->deactivate();
    }
}

void EditMode::currentDocumentTabChanged(int index)
{
    if (index == -1) return;
    docman()->setCurrentDocument(docman()->projectDocuments().at(index));
}

void EditMode::documentTabCloseRequested(int index)
{
    index = docman()->indexOf(docman()->projectDocuments().at(index));
    MainWindow::instance()->documentCloseRequested(index);
}

void EditMode::documentAdded(Document *doc)
{
    if (ProjectDocument *pdoc = doc->asProjectDocument()) {
        mDocumentStuff[doc] = new EditModePerDocumentStuff(this, pdoc);
        int docIndex = docman()->projectDocuments().indexOf(pdoc);
        mTabWidget->blockSignals(true);
        mTabWidget->insertTab(docIndex, mDocumentStuff[doc]->widget(), doc->displayName());
        mTabWidget->blockSignals(false);
        mDocumentStuff[doc]->updateDocumentTab();
    }
}

void EditMode::currentDocumentChanged(Document *doc)
{
    if (mCurrentDocumentStuff) {
        if (isActive())
            mCurrentDocumentStuff->deactivate();
    }

    mCurrentDocumentStuff = (doc && doc->isProjectDocument()) ? mDocumentStuff[doc] : 0;

    if (mCurrentDocumentStuff) {
        mTabWidget->setCurrentIndex(docman()->projectDocuments().indexOf(doc->asProjectDocument()));
        if (isActive())
            mCurrentDocumentStuff->activate();
    }
}

void EditMode::documentAboutToClose(int index, Document *doc)
{
    if (ProjectDocument *pdoc = doc->asProjectDocument()) {
        // At this point, the document is not in the DocumentManager's list of documents.
        // Removing the current tab will cause another tab to be selected and
        // the current document to change.
        // NOTE: This does not delete the tab's widget.
        QList<Document*> docs = docman()->documents();
        docs.insert(index, doc);
        for (int i = 0; i < docs.size(); i++)
            if (!doc->isProjectDocument())
                docs.takeAt(i--);
        index = docs.indexOf(doc);
        mTabWidget->removeTab(index);
    }
}
