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

#ifndef SCRIPTSDOCK_H
#define SCRIPTSDOCK_H

#include <QDockWidget>
#include <QTreeView>

class QFileSystemModel;

namespace Ui {
class ScriptsDock;
}

class ScriptsView : public QTreeView
{
    Q_OBJECT

public:
    ScriptsView(QWidget *parent = 0);

    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *event);

    QFileSystemModel *model() const { return mFSModel; }

private slots:
    void onScriptsDirectoryChanged();
    void onActivated(const QModelIndex &index);

private:
    QFileSystemModel *mFSModel;
};

class ScriptsDock : public QDockWidget
{
public:
    ScriptsDock(QWidget *parent = 0);

private:
    Ui::ScriptsDock *ui;
};

#endif // SCRIPTSDOCK_H
