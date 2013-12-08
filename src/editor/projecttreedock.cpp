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

#include "projecttreedock.h"
#include "ui_projecttreedock.h"

#include "documentmanager.h"
#include "projectactions.h"

ProjectTreeDock::ProjectTreeDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ProjectTreeDock)
{
    ui->setupUi(this);

    connect(ui->treeView, SIGNAL(activated(QModelIndex)),
            SLOT(activated(QModelIndex)));

    connect(docman(), SIGNAL(currentDocumentChanged(Document*)),
            SLOT(currentDocumentChanged(Document*)));
}

ProjectTreeDock::~ProjectTreeDock()
{
    delete ui;
}

void ProjectTreeDock::activated(const QModelIndex &index)
{
    if (BaseNode *node = ui->treeView->model()->toNode(index)) {
        ProjectActions::instance()->nodePropertiesDialog(node);
    }
}

void ProjectTreeDock::currentDocumentChanged(Document *doc)
{
    ui->treeView->model()->setDocument(doc);
    ui->treeView->expandAll();
}
