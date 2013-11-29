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

#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include "editor_global.h"

#include <QAbstractItemModel>
#include <QTreeView>

class ProjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ProjectTreeModel(QObject *parent = 0);
    ~ProjectTreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setDocument(Document *doc);

    QModelIndex index(BaseNode *object) const;

    BaseNode *toNode(const QModelIndex &index) const;

private slots:
    void afterAddNode(int index, BaseNode *node);
    void beforeRemoveNode(int index, BaseNode *node);

private:
    class Item
    {
    public:
        Item() :
            parent(0),
            node(0)
        {

        }

        Item(Item *parent, int index, BaseNode *object) :
            parent(parent),
            node(object)
        {
            parent->children.insert(index, this);
        }

        Item *parent;
        QList<Item*> children;
        BaseNode *node;
    };

    void addNode(Item *parent, BaseNode *node);
    Item *toItem(const QModelIndex &index) const;
    ProjectTreeModel::Item *toItem(BaseNode *node) const;

    QModelIndex index(Item *item) const;

    Item *mRoot;
    ProjectDocument *mDocument;
};

class ProjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = 0);

    ProjectTreeModel *model() const
    {
        return mModel;
    }

private:
    ProjectTreeModel *mModel;
};

#endif // PROJECTTREEVIEW_H
