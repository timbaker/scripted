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

#include "editnodevariabledialog.h"
#include "ui_editnodevariabledialog.h"

#include "scriptvariable.h"

EditNodeVariableDialog::EditNodeVariableDialog(ScriptVariable *var, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditNodeVariableDialog)
{
    ui->setupUi(this);

    ui->name->setText(var->name());
    ui->type->setText(var->type());
    ui->value->setText(var->value());
}

EditNodeVariableDialog::~EditNodeVariableDialog()
{
    delete ui;
}

QString EditNodeVariableDialog::value() const
{
    return ui->value->text();
}
