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
class LuaEditor;
class MetaEventDock;
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
    EditModePerDocumentStuff(EditMode *mode, Document *doc);
    ~EditModePerDocumentStuff();

    Document *document() const
    { return mDocument; }

    virtual QWidget *widget() const = 0;

    virtual void activate();
    virtual void deactivate();

public slots:
    void updateDocumentTab();

protected:
    EditMode *mMode;
    Document *mDocument;
};

class EditModePerLuaDocumentStuff : public EditModePerDocumentStuff
{
public:
    EditModePerLuaDocumentStuff(EditMode *mode, LuaDocument *doc);
    ~EditModePerLuaDocumentStuff();

    QWidget *widget() const;

    void activate();
    void deactivate();

private:
    LuaEditor *mEditor;
};

class EditModePerProjectDocumentStuff : public EditModePerDocumentStuff
{
public:
    EditModePerProjectDocumentStuff(EditMode *mode, ProjectDocument *doc);
    ~EditModePerProjectDocumentStuff();

    QWidget *widget() const;

    void activate();
    void deactivate();

private:
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
    MetaEventDock *mEventsDock;
    ScriptsDock *mScriptsDock;
    ScriptVariablesDock *mVariablesDock;
    EditModeToolBar *mToolBar;

    EditModePerDocumentStuff *mCurrentDocumentStuff;
    QMap<Document*,EditModePerDocumentStuff*> mDocumentStuff;

    friend class EditModePerDocumentStuff;
};

#endif // EDITMODE_H
