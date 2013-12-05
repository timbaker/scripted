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
    mModel->setHorizontalHeaderLabels(QStringList() << tr("From") << tr("To"));
    setModel(mModel);

    setRootIsDecorated(false);

    setDocument(docman()->currentDocument());
}

void NodeConnectionsList::setNode(BaseNode *node)
{
    mNode = node;
//    QStandardItem *rootItem = mModel->invisibleRootItem();
    mModel->removeRows(0, mModel->rowCount());
    for (int row = 0; row < node->connectionCount(); row++)
        setItems(row);
}

void NodeConnectionsList::setOutputName(const QString &outputName)
{
    for (int i = 0; i < mNode->connectionCount(); i++) {
        NodeConnection *cxn = mNode->connection(i);
        if (cxn->mOutput != outputName)
            setRowHidden(i, QModelIndex(), true);
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

        connect(mDocument->changer(), SIGNAL(afterChangeInput(NodeInput*,const NodeInput*)),
                SLOT(afterChangeInput(NodeInput*,const NodeInput*)));
        connect(mDocument->changer(), SIGNAL(afterChangeOutput(NodeOutput*,const NodeOutput*)),
                SLOT(afterChangeOutput(NodeOutput*,const NodeOutput*)));
    }
}

void NodeConnectionsList::afterAddConnection(int index, NodeConnection *cxn)
{
    if (cxn->mSender != mNode)
        return;

    mModel->insertRow(index);
    setItems(index);

    selectionModel()->select(mModel->index(index, 0),
                             QItemSelectionModel::Clear |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::SelectCurrent);
}

void NodeConnectionsList::beforeRemoveConnection(int index, NodeConnection *cxn)
{
    if (cxn->mSender != mNode)
        return;

    mModel->removeRow(index);
}

void NodeConnectionsList::afterReorderConnection(BaseNode *node, int oldIndex, int newIndex)
{
    if (node != mNode)
        return;

    //    mModel->moveRow(QModelIndex(), oldIndex, QModelIndex(), newIndex);
    mModel->removeRow(oldIndex);
    mModel->insertRow(newIndex);
    setItems(newIndex);

    selectionModel()->select(mModel->index(newIndex, 0),
                             QItemSelectionModel::Clear |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::SelectCurrent);
}

// These 2 methods only get called if there were some redo actions possible when
// a dialog using this list was displayed and the user clicked the redo button
void NodeConnectionsList::afterChangeInput(NodeInput *input, const NodeInput *oldValue)
{
    setNode(mNode);
}

void NodeConnectionsList::afterChangeOutput(NodeOutput *output, const NodeOutput *oldValue)
{
    setNode(mNode);
}

static QString inputLabel(BaseNode *node, const QString &name)
{
    if (NodeInput *input = node->input(name))
        return input->label();
    return name;
}

static QString outputLabel(BaseNode *node, const QString &name)
{
    if (NodeOutput *output = node->output(name))
        return output->label();
    return name;
}

void NodeConnectionsList::setItems(int row)
{
    NodeConnection *cxn = mNode->connection(row);

    QStandardItem *item0 = new QStandardItem(tr("%1 \"%2\"")
                                             .arg(tr(cxn->mSender->isProjectRootNode() ? "input" : "output"))
                                             .arg(cxn->mSender->isProjectRootNode()
                                                  ? inputLabel(cxn->mSender, cxn->mOutput)
                                                  : outputLabel(cxn->mSender, cxn->mOutput)));
    QStandardItem *item1 = new QStandardItem(tr("\"%1\" %2 \"%3\"")
                                             .arg(cxn->mReceiver->label())
                                             .arg(tr(cxn->mReceiver->isProjectRootNode() ? "output" : "input"))
                                             .arg(cxn->mReceiver->isProjectRootNode()
                                                  ? outputLabel(cxn->mReceiver, cxn->mInput)
                                                  : inputLabel(cxn->mReceiver, cxn->mInput)));

    item0->setEditable(false);
    item1->setEditable(false);

    bool badConnection = cxn->mReceiver->isProjectRootNode()
            ? !cxn->mReceiver->output(cxn->mInput)
            : !cxn->mReceiver->input(cxn->mInput);
    if (badConnection)
        item1->setForeground(Qt::red);

    mModel->setItem(row, 0, item0);
    mModel->setItem(row, 1, item1);
}
