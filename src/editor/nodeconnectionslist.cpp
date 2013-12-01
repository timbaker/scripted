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

#include "documentmanager.h"
#include "node.h"
#include "projectchanger.h"
#include "projectdocument.h"

NodeConnectionsList::NodeConnectionsList(QWidget *widget) :
    QTreeView(widget),
    mModel(new QStandardItemModel(this)),
    mDocument(0),
    mNode(0)
{
    mModel->setHorizontalHeaderLabels(QStringList() << tr("My Output") << tr("Target") << tr("Target Input"));
    setModel(mModel);

    setRootIsDecorated(false);

    setDocument(docman()->currentDocument());
}

void NodeConnectionsList::setNode(BaseNode *node)
{
    mNode = node;
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

void NodeConnectionsList::selectConnection(NodeConnection *cxn)
{
    int row = mNode->indexOf(cxn);
    if (row != -1) {
        QModelIndex index = mModel->index(row, 0, QModelIndex());
        /*selectionModel()->*/setCurrentIndex(index);
    }
}

void NodeConnectionsList::setDocument(Document *doc)
{
    if (mDocument)
        mDocument->changer()->disconnect(this);

    mDocument = doc ? doc->asProjectDocument() : 0;

    if (mDocument) {
        connect(mDocument->changer(), SIGNAL(afterAddConnection(int,NodeConnection*)),
                SLOT(afterAddConnection(int,NodeConnection*)));
        connect(mDocument->changer(), SIGNAL(beforeRemoveConnection(int,NodeConnection*)),
                SLOT(beforeRemoveConnection(int,NodeConnection*)));
        connect(mDocument->changer(), SIGNAL(afterReorderConnection(BaseNode*,int,int)),
                SLOT(afterReorderConnection(BaseNode*,int,int)));
    }
}

void NodeConnectionsList::afterAddConnection(int index, NodeConnection *cxn)
{
    mModel->insertRow(index);
    mModel->setItem(index, 0, new QStandardItem(cxn->mOutput));
    mModel->setItem(index, 1, new QStandardItem(cxn->mReceiver->name() + " (" + QString::number(cxn->mReceiver->id()) + ")"));
    mModel->setItem(index, 2, new QStandardItem(cxn->mInput));

    selectionModel()->select(mModel->index(index, 0),
                             QItemSelectionModel::Clear |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::SelectCurrent);
}

void NodeConnectionsList::beforeRemoveConnection(int index, NodeConnection *cxn)
{
    mModel->removeRow(index);
}

void NodeConnectionsList::afterReorderConnection(BaseNode *node, int oldIndex, int newIndex)
{
//    mModel->moveRow(QModelIndex(), oldIndex, QModelIndex(), newIndex);
    mModel->removeRow(oldIndex);
    mModel->insertRow(newIndex);
    NodeConnection *cxn = node->connection(newIndex);
    mModel->setItem(newIndex, 0, new QStandardItem(cxn->mOutput));
    mModel->setItem(newIndex, 1, new QStandardItem(cxn->mReceiver->name() + " (" + QString::number(cxn->mReceiver->id()) + ")"));
    mModel->setItem(newIndex, 2, new QStandardItem(cxn->mInput));

    selectionModel()->select(mModel->index(newIndex, 0),
                             QItemSelectionModel::Clear |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::SelectCurrent);
}
