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

#ifndef NODECONNECTIONSLIST_H
#define NODECONNECTIONSLIST_H

#include "editor_global.h"

#include <QStandardItemModel>
#include <QTreeView>

class NodeConnectionsList : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeConnectionsList(QWidget *parent = 0);

    void setNode(BaseNode *node);
    void setOutputName(const QString &outputName);
    void selectConnection(NodeConnection *cxn);

private slots:
    void setDocument(Document *doc);

    void afterAddConnection(int index, NodeConnection *cxn);
    void beforeRemoveConnection(int index, NodeConnection *cxn);
    void afterReorderConnection(BaseNode *node, int oldIndex, int newIndex);

    void afterChangeInput(NodeInput *input, const NodeInput *oldValue);
    void afterChangeOutput(NodeOutput *output, const NodeOutput *oldValue);

private:
    void setItems(int row);

private:
    QStandardItemModel *mModel;
    ProjectDocument *mDocument;
    BaseNode *mNode;
};

#endif // NODECONNECTIONSLIST_H
