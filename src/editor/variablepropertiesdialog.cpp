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

#include "variablepropertiesdialog.h"
#include "ui_variablepropertiesdialog.h"

#include "scriptvariable.h"

VariablePropertiesDialog::VariablePropertiesDialog(const ScriptVariable *var, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VariablePropertiesDialog),
    mVariable(var)
{
    ui->setupUi(this);

    ui->type->addItem(QLatin1String("Actor"));
    ui->type->addItem(QLatin1String("Boolean"));
    ui->type->addItem(QLatin1String("Integer"));
    ui->type->addItem(QLatin1String("Float"));
    ui->type->addItem(QLatin1String("String"));

    if (mVariable) {
        ui->nameEdit->setText(var->name());
        ui->type->setCurrentText(var->type());
    }
}

VariablePropertiesDialog::~VariablePropertiesDialog()
{
    delete ui;
}

QString VariablePropertiesDialog::name() const
{
    return ui->nameEdit->text();
}

QString VariablePropertiesDialog::type() const
{
    return ui->type->currentText();
}
