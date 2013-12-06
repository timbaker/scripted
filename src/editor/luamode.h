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

#ifndef LUAMODE_H
#define LUAMODE_H

#include "imode.h"

#include "editor_global.h"

#include <QMap>
#include <QToolBar>

class LuaDockWidget;
class LuaEditor;
class LuaMode;
class MetaEventDock;

class QLabel;
class QTabWidget;

class LuaModeToolBar : public QToolBar
{
    Q_OBJECT
public:
    LuaModeToolBar(QWidget *parent = 0);
};

class LuaModePerDocumentStuff : public QObject
{
    Q_OBJECT
public:
    LuaModePerDocumentStuff(LuaMode *mode, LuaDocument *doc);
    ~LuaModePerDocumentStuff();

    LuaDocument *document() const
    { return mDocument; }

    QWidget *widget() const;

    void activate();
    void deactivate();

public slots:
    void updateDocumentTab();
    void syntaxError(const QString &error);

protected:
    LuaMode *mMode;
    LuaDocument *mDocument;
    LuaEditor *mEditor;
    QWidget *mWidget;
    QLabel *mSyntaxLabel;
};

class LuaMode : public IMode
{
    Q_OBJECT
public:
    LuaMode(QObject *parent = 0);

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
    LuaModeToolBar *mToolBar;
    MetaEventDock *mEventsDock;
    LuaDockWidget *mLuaDock;

    LuaModePerDocumentStuff *mCurrentDocumentStuff;
    QMap<Document*,LuaModePerDocumentStuff*> mDocumentStuff;

    friend class LuaModePerDocumentStuff;
};

#endif // LUAMODE_H
