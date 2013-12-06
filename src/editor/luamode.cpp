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

#include "luamode.h"

#include "basegraphicsview.h"
#include "documentmanager.h"
#include "embeddedmainwindow.h"
#include "luadockwidget.h"
#include "luadocument.h"
#include "luaeditor.h"
#include "mainwindow.h"
#include "metaeventdock.h"
#include "projectactions.h"
#include "projectdocument.h"
#include "scriptscene.h"
#include "scriptview.h"
#include "toolmanager.h"

#include <QDir>
#include <QFrame>
#include <QLabel>
#include <QTabWidget>
#include <QUndoStack>
#include <QVBoxLayout>

LuaModeToolBar::LuaModeToolBar(LuaMode *mode, QWidget *parent) :
    QToolBar(parent)
{
    setWindowTitle(tr("Lua ToolBar"));

    Ui::MainWindow *actions = ProjectActions::instance()->actions();
    addAction(actions->actionNewLuaFile);
    addAction(actions->actionOpenLuaFile);
    addAction(actions->actionSaveProject);
    addAction(actions->actionSaveProjectAs);
    addSeparator();
    addAction(mode->mUndoAction);
    addAction(mode->mRedoAction);
}

/////

LuaModePerDocumentStuff::LuaModePerDocumentStuff(LuaMode *mode, LuaDocument *doc) :
    QObject(doc),
    mMode(mode),
    mDocument(doc),
    mEditor(new LuaEditor)
{
    connect(document(), SIGNAL(fileNameChanged()), SLOT(updateDocumentTab()));
    connect(document(), SIGNAL(cleanChanged()), SLOT(updateDocumentTab()));

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    mEditor->setFont(font);

    /*highlighter =*/ new Highlighter(mEditor->document());

    QFrame *frame = new QFrame;
    mWidget = frame;
    frame->setFrameShadow(mEditor->frameShadow());
    frame->setFrameShape(mEditor->frameShape());
    frame->setFrameStyle(mEditor->frameStyle());
    mEditor->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->setObjectName(QLatin1String("LuaMode.EditorVBox"));
    mSyntaxLabel = new QLabel;
    mSyntaxLabel->setFixedHeight(mSyntaxLabel->fontMetrics().lineSpacing() * 2);
    mSyntaxLabel->setVisible(false);
    mSyntaxLabel->setFont(font);
    QFrame *seperator = new QFrame;
    seperator->setFrameStyle(QFrame::HLine);
    vbox->addWidget(mSyntaxLabel);
    vbox->addWidget(seperator);
    vbox->addWidget(mEditor);
    mWidget->setLayout(vbox);

    connect(mEditor, SIGNAL(syntaxError(QString)), SLOT(syntaxError(QString)));

    doc->setEditor(mEditor); // a bit kludgey
}

LuaModePerDocumentStuff::~LuaModePerDocumentStuff()
{
    // widget() is added to a QTabWidget.
    // Removing a tab does not delete the page widget.
    delete widget();
}

QWidget *LuaModePerDocumentStuff::widget() const
{
    return mWidget;
}

void LuaModePerDocumentStuff::activate()
{
    ToolManager::instance()->setScene(0);

    connect(mMode->mUndoAction, SIGNAL(triggered()), mEditor, SLOT(undo()));
    connect(mMode->mRedoAction, SIGNAL(triggered()), mEditor, SLOT(redo()));

    connect(mEditor, SIGNAL(undoAvailable(bool)), mMode, SLOT(updateUndoAction(bool)));
    connect(mEditor, SIGNAL(redoAvailable(bool)), mMode, SLOT(updateRedoAction(bool)));

    mMode->updateUndoAction(mEditor->document()->isUndoAvailable());
    mMode->updateRedoAction(mEditor->document()->isRedoAvailable());
}

void LuaModePerDocumentStuff::deactivate()
{
    disconnect(mMode->mUndoAction, SIGNAL(triggered()), mEditor, SLOT(undo()));
    disconnect(mMode->mRedoAction, SIGNAL(triggered()), mEditor, SLOT(redo()));

    disconnect(mEditor, SIGNAL(undoAvailable(bool)), mMode, SLOT(updateUndoAction(bool)));
    disconnect(mEditor, SIGNAL(redoAvailable(bool)), mMode, SLOT(updateRedoAction(bool)));
}

void LuaModePerDocumentStuff::updateDocumentTab()
{
    int tabIndex = docman()->luaDocuments().indexOf(document());
    if (tabIndex == -1)
        return;

    QString tabText = document()->displayName();
    if (document()->isModified())
        tabText.prepend(QLatin1Char('*'));
    mMode->mTabWidget->setTabText(tabIndex, tabText);

    QString tooltipText = QDir::toNativeSeparators(document()->fileName());
    mMode->mTabWidget->setTabToolTip(tabIndex, tooltipText);
}

void LuaModePerDocumentStuff::syntaxError(const QString &error)
{
    if (error.isEmpty()) {
        mSyntaxLabel->setVisible(false);
    } else {
        mSyntaxLabel->setText(error);
        mSyntaxLabel->setVisible(true);
    }
}

/////

LuaMode::LuaMode(QObject *parent) :
    IMode(parent),
    mEventsDock(new MetaEventDock),
    mLuaDock(new LuaDockWidget),
    mUndoAction(new QAction(tr("Undo"), this)),
    mRedoAction(new QAction(tr("Redo"), this)),
    mToolBar(new LuaModeToolBar(this)),
    mCurrentDocumentStuff(0)
{
    setDisplayName(tr("Lua"));
    setIcon(QIcon(QLatin1String(":images/16x16/lua-mode.png")));

    mEventsDock->disableDragAndDrop();
    mLuaDock->disableDragAndDrop();

    mMainWindow = new EmbeddedMainWindow;
    mMainWindow->setObjectName(QLatin1String("LuaMode.Widget"));

    mTabWidget = new QTabWidget;
    mTabWidget->setObjectName(QLatin1String("LuaMode.TabWidget"));
    mTabWidget->setDocumentMode(true);
    mTabWidget->setTabsClosable(true);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setObjectName(QLatin1String("LuaMode.VBox"));
    vbox->setMargin(0);
    vbox->addWidget(mTabWidget);
//    vbox->addLayout(mStatusBar->statusBarLayout);
    vbox->setStretchFactor(mTabWidget, 1);

    QWidget *w = new QWidget;
    w->setObjectName(QLatin1String("LuaMode.VBoxWidget"));
    w->setLayout(vbox);

    mMainWindow->setCentralWidget(w);
    mMainWindow->addToolBar(mToolBar);

    mMainWindow->registerDockWidget(mEventsDock);
    mMainWindow->registerDockWidget(mLuaDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mEventsDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mLuaDock);

    setWidget(mMainWindow);

#if 1
    mUndoAction->setShortcuts(QKeySequence::Undo);
    mRedoAction->setShortcuts(QKeySequence::Redo);
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));
    undoIcon.addFile(QLatin1String(":images/24x24/edit-undo.png"));
    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    redoIcon.addFile(QLatin1String(":images/24x24/edit-redo.png"));
    mUndoAction->setIcon(undoIcon);
    mRedoAction->setIcon(redoIcon);
    mUndoAction->setVisible(false);
    mRedoAction->setVisible(false);

    Ui::MainWindow *actions = ProjectActions::instance()->actions();
    actions->menuEdit->insertAction(ProjectActions::instance()->undoAction(), mRedoAction);
    actions->menuEdit->insertAction(mRedoAction, mUndoAction);
#endif

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

void LuaMode::readSettings(QSettings &settings)
{
    settings.beginGroup(QLatin1String("LuaMode"));
    mMainWindow->readSettings(settings);
    settings.endGroup();
}

void LuaMode::writeSettings(QSettings &settings)
{
    settings.beginGroup(QLatin1String("LuaMode"));
    mMainWindow->writeSettings(settings);
    settings.endGroup();
}

void LuaMode::onActiveStateChanged(bool active)
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

    mUndoAction->setVisible(active);
    mRedoAction->setVisible(active);
}

void LuaMode::currentDocumentTabChanged(int index)
{
    if (index == -1) return;
    docman()->setCurrentDocument(docman()->luaDocuments().at(index));
}

void LuaMode::documentTabCloseRequested(int index)
{
    index = docman()->indexOf(docman()->luaDocuments().at(index));
    MainWindow::instance()->documentCloseRequested(index);
}

void LuaMode::documentAdded(Document *doc)
{
    if (LuaDocument *ldoc = doc->asLuaDocument()) {
        mDocumentStuff[doc] = new LuaModePerDocumentStuff(this, ldoc);
        int docIndex = docman()->luaDocuments().indexOf(ldoc);
        mTabWidget->blockSignals(true);
        mTabWidget->insertTab(docIndex, mDocumentStuff[doc]->widget(), doc->displayName());
        mTabWidget->blockSignals(false);
        mDocumentStuff[doc]->updateDocumentTab();
    }
}

void LuaMode::currentDocumentChanged(Document *doc)
{
    if (mCurrentDocumentStuff) {
        if (isActive())
            mCurrentDocumentStuff->deactivate();
    }

    mCurrentDocumentStuff = (doc && doc->isLuaDocument()) ? mDocumentStuff[doc] : 0;

    if (mCurrentDocumentStuff) {
        mTabWidget->setCurrentIndex(docman()->luaDocuments().indexOf(doc->asLuaDocument()));
        if (isActive())
            mCurrentDocumentStuff->activate();
    }
}

void LuaMode::documentAboutToClose(int index, Document *doc)
{
    if (LuaDocument *ldoc = doc->asLuaDocument()) {
        // At this point, the document is not in the DocumentManager's list of documents.
        // Removing the current tab will cause another tab to be selected and
        // the current document to change.
        // NOTE: This does not delete the tab's widget.
        QList<Document*> docs = docman()->documents();
        docs.insert(index, doc);
        for (int i = 0; i < docs.size(); i++)
            if (!doc->isLuaDocument())
                docs.takeAt(i--);
        index = docs.indexOf(doc);
        mTabWidget->removeTab(index);
    }
}

void LuaMode::updateUndoAction(bool enable)
{
    mUndoAction->setEnabled(enable);
}

void LuaMode::updateRedoAction(bool enable)
{
    mRedoAction->setEnabled(enable);
}
