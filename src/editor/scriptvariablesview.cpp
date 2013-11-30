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

#include "scriptvariablesview.h"

#include "documentmanager.h"
#include "node.h"
#include "project.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "scriptvariable.h"

#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QStandardItemModel>

class ScriptVariablesDelegate : public QAbstractItemDelegate
{
public:
    ScriptVariablesDelegate(ScriptVariablesView *view, QObject *parent = 0)
        : QAbstractItemDelegate(parent)
        , mView(view)
#if 0
        , mLabelFontMetrics(mLabelFont)
#endif
    {
#if 0
        if (true/*mFont != mView->font()*/) {
            mFont = mView->font();
            mLabelFont = mFont.resolve(mFont);
            mLabelFont.setBold(true);
            if (mLabelFont.pixelSize() != -1)
                mLabelFont.setPixelSize(mLabelFont.pixelSize() + 4);
            else if (mLabelFont.pointSize() != -1)
                mLabelFont.setPointSize(mLabelFont.pointSize() + 1);
            mLabelFontMetrics = QFontMetrics(mLabelFont);
        }
#endif
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    static const int INSET = 3;
    static const int BOXSIZE = 48;
private:
    ScriptVariablesView *mView;
#if 0
    QFont mFont;
    QFont mLabelFont;
    QFontMetrics mLabelFontMetrics;
#endif
};

void ScriptVariablesDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    painter->fillRect(option.rect.x() + (option.rect.width() - BOXSIZE) / 2, option.rect.y() + INSET,
                      BOXSIZE, BOXSIZE, Qt::gray);

//    painter->setFont(mLabelFont);
    painter->drawText(option.rect.x(), option.rect.y() + INSET + BOXSIZE,
                      option.rect.width(), option.fontMetrics.lineSpacing(),
                      Qt::AlignHCenter,
                      index.data(Qt::DisplayRole).toString());

    if (option.state & QStyle::State_Selected) {
        QPen oldPen = painter->pen();
        QPen pen;
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawRect(option.rect.adjusted(1,1,-1,-1));
        painter->setPen(oldPen);
    }
}

QSize ScriptVariablesDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option)
#if 1
    if (index.parent().isValid())
        return QSize();
    return QSize(qMax(BOXSIZE, option.fontMetrics.boundingRect(index.data(Qt::DisplayRole).toString()).width()) + INSET * 2,
                 BOXSIZE + INSET * 2 + option.fontMetrics.lineSpacing());
#else
    const ScriptVariablesModel *m = static_cast<const ScriptVariablesModel*>(index.model());
    QVector<BmpBlend*> blends = m->blendsAt(index);
    if (blends.isEmpty()) return QSize();

    const qreal zoom = mView->zoomable()->scale();
    const int extra = 2 * 2;

    const QFontMetrics &fm = mLabelFontMetrics;
    const int labelHeight = fm.lineSpacing();

    const int tileWidth = qCeil(64 * zoom);
    const int tileHeight = mView->isExpanded() ? qCeil(128 * zoom) : 0;
    const int viewWidth = mView->viewport()->width();
    const int columns = 9;
    return QSize(qMax(qMax(tileWidth * columns, mView->maxHeaderWidth()) + extra, viewWidth),
                 2 + labelHeight + (tileHeight * 1) + extra);
#endif
}

/////

ScriptVariablesModel::ScriptVariablesModel(QObject *parent) :
    QAbstractItemModel(parent),
    mDocument(0)
{
    connect(docman(), SIGNAL(currentDocumentChanged(Document*)),
            SLOT(setDocument(Document*)));
}

ScriptVariablesModel::~ScriptVariablesModel()
{
    qDeleteAll(mItems);
}

int ScriptVariablesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mItems.size();
}

int ScriptVariablesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

Qt::ItemFlags ScriptVariablesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (itemAt(index))
        flags |= Qt::ItemIsDragEnabled;
    return flags;
}

QVariant ScriptVariablesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (Item *item = itemAt(index)) {
            return item->mVariable->name();
        }
    }
#if 0
    if (role == Qt::ToolTipRole) {
        if (Item *item = toItem(index)) {
            return item->mVariable->mName;
        }
    }
#endif
    return QVariant();
}

QVariant ScriptVariablesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

QModelIndex ScriptVariablesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || (row >= mItems.size()))
        return QModelIndex();

    return createIndex(row, column, mItems.at(row));
}

QModelIndex ScriptVariablesModel::index(ScriptVariable *var)
{
    if (Item *item = itemFor(var))
        return index(mItems.indexOf(item), 0, QModelIndex());
    return QModelIndex();
}

QModelIndex ScriptVariablesModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

static QString VARIABLE_MIME_TYPE = QLatin1String("application/x-pzdraft-variable");

QStringList ScriptVariablesModel::mimeTypes() const
{
    return QStringList() << VARIABLE_MIME_TYPE;
}

QMimeData *ScriptVariablesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach (const QModelIndex &index, indexes) {
        if (Item *item = itemAt(index)) {
            stream << item->mVariable->name();
        }
    }

    mimeData->setData(VARIABLE_MIME_TYPE, encodedData);
    return mimeData;
}

ScriptVariable *ScriptVariablesModel::variableAt(const QModelIndex &index)
{
    if (Item *item = itemAt(index))
        return item->mVariable;
    return 0;
}

void ScriptVariablesModel::reset()
{
    if (mItems.size()) {
        // Do this because beginResetModel() doesn't update the selection
        beginRemoveRows(QModelIndex(), 0, mItems.size());
        qDeleteAll(mItems);
        mItems.clear();
        endRemoveRows();
    }

    if (!mDocument)
        return;

    if (int count = mDocument->project()->rootNode()->variableCount()) {
        beginInsertRows(QModelIndex(), 0, count - 1);
        foreach (ScriptVariable *var, mDocument->project()->rootNode()->variables()) {
            Item *item = new Item(var);
            mItems += item;
        }
        endInsertRows();
    }
}

void ScriptVariablesModel::setDocument(Document *doc)
{
    if (mDocument)
        mDocument->changer()->disconnect(this);

    mDocument = doc ? doc->asProjectDocument() : 0;

    if (mDocument) {
        connect(mDocument->changer(), SIGNAL(afterAddVariable(int,ScriptVariable*)),
               SLOT(afterAddVariable(int,ScriptVariable*)));
        connect(mDocument->changer(), SIGNAL(beforeRemoveVariable(int,ScriptVariable*)),
                SLOT(beforeRemoveVariable(int,ScriptVariable*)));
        connect(mDocument->changer(), SIGNAL(afterChangeVariable(ScriptVariable*,const ScriptVariable*)),
                SLOT(afterChangeVariable(ScriptVariable*,const ScriptVariable*)));
    }

    reset();
}

void ScriptVariablesModel::afterAddVariable(int index, ScriptVariable *var)
{
    beginInsertRows(QModelIndex(), index, index);
    mItems.insert(index, new Item(var));
    endInsertRows();
}

void ScriptVariablesModel::beforeRemoveVariable(int index, ScriptVariable *var)
{
    beginRemoveRows(QModelIndex(), index, index);
    delete mItems.takeAt(index);
    endRemoveRows();
}

void ScriptVariablesModel::afterChangeVariable(ScriptVariable *var, const ScriptVariable *oldValue)
{
    if (!itemFor(var))
        return; // could be a variable of a child node
    Q_UNUSED(oldValue)
    emit dataChanged(index(var), index(var));
}

ScriptVariablesModel::Item *ScriptVariablesModel::itemAt(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer());
    return 0;
}

ScriptVariablesModel::Item *ScriptVariablesModel::itemFor(ScriptVariable *var)
{
    foreach (Item *item, mItems)
        if (item->mVariable == var)
            return item;
    return 0;
}

/////

ScriptVariablesView::ScriptVariablesView(QWidget *parent) :
    QListView(parent),
    mModel(new ScriptVariablesModel(this))
{
    setFlow(LeftToRight);
//    setViewMode(IconMode);
//    setUniformItemSizes(true);
    setModel(mModel);
    // item-delegate's size hint sets correct width but the item height is always
    // the height of the list view ???
//    setGridSize(QSize(64, 64));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QStyleOptionViewItem option;
    option.init(this);

    int itemHeight = ScriptVariablesDelegate::BOXSIZE + ScriptVariablesDelegate::INSET * 2;
    itemHeight += option.fontMetrics.lineSpacing();
    setFixedHeight(itemHeight + frameWidth() * 2 + horizontalScrollBar()->height());

    setDragDropMode(QAbstractItemView::DragOnly);

    setItemDelegate(new ScriptVariablesDelegate(this, this));
}
