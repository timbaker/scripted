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

#include "scriptvariablesdock.h"
#include "ui_scriptvariablesdock.h"

#include "project.h"
#include "projectactions.h"
#include "projectdocument.h"
#include "scriptvariable.h"

ScriptVariablesDock::ScriptVariablesDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScriptVariablesDock),
    mCurrentVar(0)
{
    ui->setupUi(this);

    connect(ui->listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));
    connect(ui->listView, SIGNAL(activated(QModelIndex)), SLOT(activated(QModelIndex)));

    connect(ui->add, SIGNAL(clicked()), ProjectActions::instance(), SLOT(addVariable()));
    connect(ui->properties, SIGNAL(clicked()), SLOT(properties()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeVariable()));

    syncUI();
}

ScriptVariablesDock::~ScriptVariablesDock()
{
    delete ui;
}

void ScriptVariablesDock::selectionChanged()
{
    mCurrentVar = 0;
    if (ui->listView->selectionModel()->selectedIndexes().size() == 1) {
        QModelIndex index = ui->listView->selectionModel()->selectedIndexes().first();
        mCurrentVar = ui->listView->model()->variableAt(index);
    }
    syncUI();
}

void ScriptVariablesDock::activated(const QModelIndex &index)
{
    properties();
}

void ScriptVariablesDock::properties()
{
    if (!mCurrentVar)
        return;
    ProjectActions::instance()->variableProperties(mCurrentVar);
}

void ScriptVariablesDock::removeVariable()
{
    if (!mCurrentVar)
        return;
    ProjectActions::instance()->removeVariable(mCurrentVar);
}

void ScriptVariablesDock::syncUI()
{
    ui->properties->setEnabled(mCurrentVar != 0);
    ui->remove->setEnabled(mCurrentVar != 0);
}
