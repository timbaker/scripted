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

#include "projecttreeview.h"

#include "node.h"
#include "project.h"
#include "projectchanger.h"
#include "projectdocument.h"

#include <QFileInfo>

ProjectTreeModel::ProjectTreeModel(QObject *parent) :
    QAbstractItemModel(parent),
    mRoot(0),
    mDocument(0)
{
}

ProjectTreeModel::~ProjectTreeModel()
{
    delete mRoot;
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!mRoot)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row < mRoot->children.size())
            return createIndex(row, column, mRoot->children.at(row));
        return QModelIndex();
    }

    if (Item *item = toItem(parent)) {
        if (row < item->children.size())
            return createIndex(row, column, item->children.at(row));
        return QModelIndex();
    }

    return QModelIndex();
}

QModelIndex ProjectTreeModel::parent(const QModelIndex &index) const
{
    if (Item *item = toItem(index)) {
        if (item->parent != mRoot) {
            Item *grandParent = item->parent->parent;
            return createIndex(grandParent->children.indexOf(item->parent), 0, item->parent);
        }
    }
    return QModelIndex();
}

int ProjectTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!mRoot)
        return 0;
    if (!parent.isValid())
        return mRoot->children.size();
    if (Item *item = toItem(parent))
        return item->children.size();
    return 0;
}

int ProjectTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ProjectTreeModel::data(const QModelIndex &index, int role) const
{
    Item *item = toItem(index);
    if (!item)
        return QVariant();

    if (BaseNode *node = item->node) {
        switch (role) {
        case Qt::DisplayRole:
            return node->name();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool ProjectTreeModel::setData(const QModelIndex &index, const QVariant &value,
                               int role)
{
    Item *item = toItem(index);
    if (!item)
        return false;

    return false;
}

Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    return rc;
}

QVariant ProjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Name");
        }
    }
    return QVariant();
}

QModelIndex ProjectTreeModel::index(BaseNode *node) const
{
    Item *item = toItem(node);
    return index(item);
}

BaseNode *ProjectTreeModel::toNode(const QModelIndex &index) const
{
    if (Item *item = toItem(index))
        return item->node;
    return 0;
}

void ProjectTreeModel::afterAddNode(int index, BaseNode *node)
{
    Item *projectItem = mRoot->children.first();
    beginInsertRows(this->index(projectItem), index, index);
    new Item(projectItem, index, node);
    endInsertRows();
}

void ProjectTreeModel::beforeRemoveNode(int index, BaseNode *node)
{
    Item *projectItem = mRoot->children.first();
    beginRemoveRows(this->index(projectItem), index, index);
    delete projectItem->children.takeAt(index);
    endRemoveRows();
}

void ProjectTreeModel::afterRenameNode(BaseNode *node, const QString &oldName)
{
    Q_UNUSED(oldName);
    QModelIndex index = index(node);
    Q_ASSERT(index.isValid());
    emit dataChanged(index, index);
}

void ProjectTreeModel::addNode(Item *parent, BaseNode *node)
{
    Item *item = new Item(parent, parent->children.size(), node);
    if (ScriptNode *snode = node->asScriptNode())
        foreach (BaseNode *child, snode->nodes())
            new Item(item, item->children.size(), child);
}

void ProjectTreeModel::setDocument(Document *doc)
{
    beginResetModel();

    delete mRoot;
    mRoot = 0;

    if (mDocument) {
        mDocument->changer()->disconnect(this);
    }

    mDocument = doc ? doc->asProjectDocument() :0;

    if (mDocument) {
        mRoot = new Item;
        addNode(mRoot, mDocument->project()->rootNode());

        connect(mDocument->changer(), SIGNAL(afterAddNode(int,BaseNode*)),
                SLOT(afterAddNode(int,BaseNode*)));
        connect(mDocument->changer(), SIGNAL(beforeRemoveNode(int,BaseNode*)),
                SLOT(beforeRemoveNode(int,BaseNode*)));
        connect(mDocument->changer(), SIGNAL(afterRenameNode(BaseNode*,QString)),
                SLOT(afterRenameNode(BaseNode*,QString)));
    }

    endResetModel();
}

ProjectTreeModel::Item *ProjectTreeModel::toItem(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer());
    return 0;
}

ProjectTreeModel::Item *ProjectTreeModel::toItem(BaseNode *node) const
{
    if (!mRoot)
        return 0;
    foreach (Item *item, mRoot->children)
        if (item->node == node)
            return item;
    return 0;
}

QModelIndex ProjectTreeModel::index(Item *item) const
{
    int row = item->parent->children.indexOf(item);
    return createIndex(row, 0, item);
}

/////

ProjectTreeView::ProjectTreeView(QWidget *parent) :
    QTreeView(parent),
    mModel(new ProjectTreeModel)
{
    setRootIsDecorated(true);
    setHeaderHidden(true);
    setItemsExpandable(true);
    setUniformRowHeights(true);

    setSelectionBehavior(QAbstractItemView::SelectRows);

    setModel(mModel);
}
