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

#ifndef EDITNODEVARIABLEDIALOG_H
#define EDITNODEVARIABLEDIALOG_H

#include "editor_global.h"
#include <QDialog>

namespace Ui {
class EditNodeVariableDialog;
}

class EditNodeVariableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditNodeVariableDialog(ScriptVariable *var, QWidget *parent = 0);
    ~EditNodeVariableDialog();

    QString value() const;

private:
    Ui::EditNodeVariableDialog *ui;
};

#endif // EDITNODEVARIABLEDIALOG_H
