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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editor_global.h"
#include "singleton.h"

#include <QMainWindow>
#include <QMap>
#include <QSettings>

class EditMode;
class IMode;
class LuaMode;
class WelcomeMode;

namespace Core {
namespace Internal {
class FancyTabWidget;
}
}

class QSettings;
class QUndoGroup;

namespace Ui {
class MainWindow;
}

class PerDocumentStuff : public QObject
{
    Q_OBJECT
public:
    PerDocumentStuff(Document *doc);

    Document *document() const
    { return mDocument; }

    void rememberTool();
    void restoreTool();

    Document *mDocument;
    AbstractTool *mPrevTool;
};

class MainWindow : public QMainWindow, public Singleton<MainWindow>
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

    bool confirmAllSave();
    bool confirmSave();

    void writeSettings();
    void readSettings();

    void openLastFiles();

    void switchToEditMode();
    void switchToLuaMode();

public slots:
    void documentAdded(Document *doc);
    void currentDocumentChanged(Document *doc);
    void documentCloseRequested(int index);

    void currentModeAboutToChange(IMode *mode);
    void currentModeChanged();

private:
    Ui::MainWindow *ui;
    Core::Internal::FancyTabWidget *mTabWidget;
    WelcomeMode *mWelcomeMode;
    EditMode *mEditMode;
    LuaMode *mLuaMode;

    QMap<Document*,PerDocumentStuff*> mDocumentStuff;
    bool mDocumentChanging;
    PerDocumentStuff *mCurrentDocumentStuff;

    QSettings mSettings;
    QUndoGroup *mUndoGroup;
};

inline MainWindow *mainwin() { return MainWindow::instance(); }

#endif // MAINWINDOW_H
