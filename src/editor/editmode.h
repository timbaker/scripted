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

#ifndef EDITMODE_H
#define EDITMODE_H

#include "imode.h"

#include "editor_global.h"

#include <QMap>
#include <QToolBar>

class LuaDockWidget;
class ProjectTreeDock;
class ScriptsDock;
class ScriptVariablesDock;

class QTabWidget;

class EditMode;

class EditModeToolBar : public QToolBar
{
    Q_OBJECT
public:
    EditModeToolBar(QWidget *parent = 0);
};

class EditModePerDocumentStuff : public QObject
{
    Q_OBJECT
public:
    EditModePerDocumentStuff(EditMode *mode, ProjectDocument *doc);
    ~EditModePerDocumentStuff();

    ProjectDocument *document() const
    { return mDocument; }

    ProjectView *view() const
    { return mView; }

    void activate();
    void deactivate();

public slots:
    void updateDocumentTab();

private:
    EditMode *mMode;
    ProjectDocument *mDocument;
    ProjectScene *mScene;
    ProjectView *mView;
};

class EditMode : public IMode
{
    Q_OBJECT
public:
    EditMode(QObject *parent = 0);

    void readSettings(QSettings &settings);
    void writeSettings(QSettings &settings);

private slots:
    void onActiveStateChanged(bool active);
    void currentDocumentTabChanged(int index);
    void documentTabCloseRequested(int index);

    void documentAdded(Document *doc);
    void currentDocumentChanged(Document *doc);
    void documentAboutToClose(int index, Document *doc);

protected:
    EmbeddedMainWindow *mMainWindow;
    QTabWidget *mTabWidget;
    ProjectTreeDock *mProjectDock;
    LuaDockWidget *mLuaDock;
    ScriptsDock *mScriptsDock;
    ScriptVariablesDock *mVariablesDock;
    EditModeToolBar *mToolBar;

    EditModePerDocumentStuff *mCurrentDocumentStuff;
    QMap<Document*,EditModePerDocumentStuff*> mDocumentStuff;

    friend class EditModePerDocumentStuff;
};

#endif // EDITMODE_H
