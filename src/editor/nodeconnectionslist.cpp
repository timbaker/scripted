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

#include "nodeconnectionslist.h"

#include "node.h"

NodeConnectionsList::NodeConnectionsList(QWidget *widget) :
    QTreeView(widget),
    mModel(new QStandardItemModel(this))
{
    mModel->setHorizontalHeaderLabels(QStringList() << tr("My Output") << tr("Target") << tr("Target Input"));
    setModel(mModel);

    setRootIsDecorated(false);
}

void NodeConnectionsList::setNode(BaseNode *node)
{
//    QStandardItem *rootItem = mModel->invisibleRootItem();
    mModel->removeRows(0, mModel->rowCount());
    int row = 0;
    foreach (NodeConnection *cxn, node->connections()) {
       mModel->setItem(row, 0, new QStandardItem(cxn->mOutput));
       mModel->setItem(row, 1, new QStandardItem(cxn->mReceiver->name() + " (" + QString::number(cxn->mReceiver->id()) + ")"));
       mModel->setItem(row, 2, new QStandardItem(cxn->mInput));
       row++;
    }
}
