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

#ifndef SCRIPTVARIABLESDOCK_H
#define SCRIPTVARIABLESDOCK_H

#include "editor_global.h"
#include <QDockWidget>

namespace Ui {
class ScriptVariablesDock;
}

class ScriptVariablesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ScriptVariablesDock(QWidget *parent = 0);
    ~ScriptVariablesDock();

private slots:
    void selectionChanged();
    void activated(const QModelIndex &index);
    void properties();
    void removeVariable();
    void syncUI();

private:
    Ui::ScriptVariablesDock *ui;
    ScriptVariable *mCurrentVar;
};

#endif // SCRIPTVARIABLESDOCK_H
