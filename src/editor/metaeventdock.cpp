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

#include "metaeventdock.h"
#include "ui_metaeventdock.h"

#include "node.h"
#include "metaeventmanager.h"
#include "preferences.h"

#include <QMimeData>

const QString METAEVENT_MIME_TYPE = QLatin1String("application/x-pzdraft-event");

MetaEventModel::MetaEventModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

QStringList MetaEventModel::mimeTypes() const
{
    return QStringList() << METAEVENT_MIME_TYPE;
}

QMimeData *MetaEventModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach (const QModelIndex &index, indexes) {
        if (QStandardItem *item = itemFromIndex(index)) {
            stream << item->text();
        }
    }

    mimeData->setData(METAEVENT_MIME_TYPE, encodedData);
    return mimeData;
}

/////

MetaEventDock::MetaEventDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MetaEventDock),
    mModel(new MetaEventModel(this))
{
    ui->setupUi(this);

    ui->treeView->setModel(mModel);
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setDragEnabled(true);

    connect(prefs(), SIGNAL(gameDirectoriesChanged()), SLOT(setList()));
    connect(eventmgr(), SIGNAL(infoChanged(MetaEventInfo*)), SLOT(setList())); // FIXME: multiple calls

    setList();
}

MetaEventDock::~MetaEventDock()
{
    delete ui;
}

void MetaEventDock::setList()
{
    mModel->setRowCount(0);
    foreach (MetaEventInfo *info, eventmgr()->events()) {
        if (!info->node()) continue;
        QList<QStandardItem*> items;
        items += new QStandardItem(info->node() ? info->node()->name() : tr("FIXME"));
        mModel->appendRow(items);
    }
}

