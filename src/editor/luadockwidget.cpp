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

#include "luadockwidget.h"
#include "ui_luadockwidget.h"

#include "luamanager.h"
#include "node.h"

#include <QMimeData>

LuaDockWidget::LuaDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::LuaDockWidget)
{
    ui->setupUi(this);

    ui->listView->setModel(new LuaItemModel(ui->listView));
    ui->listView->setDragEnabled(true);
    ui->listView->setDragDropMode(QAbstractItemView::DragOnly);

    setList();
}

LuaDockWidget::~LuaDockWidget()
{
    delete ui;
}

void LuaDockWidget::setList()
{
    ((LuaItemModel*)ui->listView->model())->reset();
}

/////

LuaItemModel::LuaItemModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

LuaItemModel::~LuaItemModel()
{
    qDeleteAll(mItems);
}

int LuaItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mItems.size();
}

int LuaItemModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

Qt::ItemFlags LuaItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (itemAt(index))
        flags |= Qt::ItemIsDragEnabled;
    return flags;
}

QVariant LuaItemModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (Item *item = itemAt(index)) {
            return item->mDefinition->node()->name();
        }
    }
#if 0
    if (role == Qt::ToolTipRole) {
        if (Item *item = toItem(index)) {
            return item->mDefinition->mName;
        }
    }
#endif
    return QVariant();
}

QVariant LuaItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

QModelIndex LuaItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || mItems.isEmpty())
        return QModelIndex();

    return createIndex(row, column, mItems.at(row));
}

QModelIndex LuaItemModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

static QString COMMAND_MIME_TYPE = QLatin1String("application/x-pzdraft-command");

QStringList LuaItemModel::mimeTypes() const
{
    return QStringList() << COMMAND_MIME_TYPE;
}

QMimeData *LuaItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach (const QModelIndex &index, indexes) {
        if (Item *item = itemAt(index)) {
            stream << item->mDefinition->path(); // FIXME: path/to/file.lua
        }
    }

    mimeData->setData(COMMAND_MIME_TYPE, encodedData);
    return mimeData;
}

void LuaItemModel::reset()
{
    beginResetModel();
    qDeleteAll(mItems);
    foreach (LuaInfo *def, luamgr()->commands()) {
        Item *item = new Item(def);
        mItems += item;
    }
    endResetModel();
}

LuaItemModel::Item *LuaItemModel::itemAt(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer());
    return 0;
}
