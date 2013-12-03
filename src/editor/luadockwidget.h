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

#ifndef LUADOCKWIDGET_H
#define LUADOCKWIDGET_H

#include "editor_global.h"

#include <QDockWidget>
#include <QAbstractItemModel>

namespace Ui {
class LuaDockWidget;
}

class LuaItemModel : public QAbstractItemModel
{
public:
    LuaItemModel(QObject *parent);
    ~LuaItemModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

    void reset();

private:
    class Item
    {
    public:
        Item() :
            mDefinition(0)
        {
        }

        Item(LuaInfo *def) :
            mDefinition(def)
        {
        }

        LuaInfo *mDefinition;
    };

    Item *itemAt(const QModelIndex &index) const;
    QList<Item*> mItems;
};

class LuaDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit LuaDockWidget(QWidget *parent = 0);
    ~LuaDockWidget();

private slots:
    void setList();

private:
    Ui::LuaDockWidget *ui;
};

#endif // LUADOCKWIDGET_H
