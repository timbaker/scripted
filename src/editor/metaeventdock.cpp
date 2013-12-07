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
#include "projectactions.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeData>

Q_DECLARE_METATYPE(MetaEventInfo*)

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
            MetaEventInfo *info = item->data().value<MetaEventInfo*>();
            stream << info->eventName() << info->path();
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

    mTimer.setInterval(500);
    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), SLOT(setList()));

    connect(ui->dirComboBox, SIGNAL(currentIndexChanged(int)), SLOT(dirSelected(int)));
    connect(ui->treeView, SIGNAL(activated(QModelIndex)), SLOT(activated(QModelIndex)));

    connect(prefs(), SIGNAL(gameDirectoriesChanged()), SLOT(setList()));
    connect(eventmgr(), SIGNAL(infoChanged(MetaEventInfo*)), &mTimer, SLOT(start()));

    setDirCombo();
}

MetaEventDock::~MetaEventDock()
{
    delete ui;
}

void MetaEventDock::disableDragAndDrop()
{
    ui->treeView->setDragEnabled(false);
}

void MetaEventDock::setList()
{
    mModel->setRowCount(0);
    if (ui->dirComboBox->currentIndex() != -1) {
        QString fileName = prefs()->gameDirectories().at(ui->dirComboBox->currentIndex());
        fileName = QDir(fileName).filePath(QLatin1String("media/lua/MetaGame/MetaEvents.lua"));
        foreach (MetaEventInfo *info, eventmgr()->events(fileName)) {
            if (!info->node()) continue;
            QList<QStandardItem*> items;
            QStandardItem *item = new QStandardItem(info->eventName());
            item->setEditable(false);
            item->setData(QVariant::fromValue(info));
            items += item;
            mModel->appendRow(items);
        }
    }
}

void MetaEventDock::activated(const QModelIndex &index)
{
    QStandardItem *item = mModel->itemFromIndex(index);
    MetaEventInfo *info = item->data().value<MetaEventInfo*>();
    ProjectActions::instance()->openLuaFile(info->path());
}

void MetaEventDock::dirSelected(int index)
{
    setList();
}

void MetaEventDock::setDirCombo()
{
    ui->dirComboBox->clear();
    foreach (QString path, prefs()->gameDirectories()) {
        QFileInfo info(path);
        ui->dirComboBox->addItem(info.fileName());
    }
}

