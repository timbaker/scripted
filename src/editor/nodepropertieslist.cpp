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

#include "nodepropertieslist.h"

#include "node.h"
#include "scriptvariable.h"

NodePropertiesList::NodePropertiesList(QWidget *widget) :
    QTreeView(widget),
    mModel(new QStandardItemModel(this))
{
    mModel->setColumnCount(2);
    mModel->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
    setModel(mModel);

    setRootIsDecorated(false);
}

void NodePropertiesList::setNode(BaseNode *node)
{
//    QStandardItem *rootItem = mModel->invisibleRootItem();
    mModel->removeRows(0, mModel->rowCount());
    int row = 0;
    foreach (ScriptVariable *v, node->variables()) {
       mModel->setItem(row, 0, new QStandardItem(v->name()));
       mModel->setItem(row, 1, new QStandardItem(v->value()));
       row++;
    }
}
