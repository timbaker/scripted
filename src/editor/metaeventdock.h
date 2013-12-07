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

#ifndef METAEVENTDOCK_H
#define METAEVENTDOCK_H

#include <QDockWidget>
#include <QStandardItemModel>
#include <QTimer>

class MetaEventModel : public QStandardItemModel
{
public:
    MetaEventModel(QObject *parent = 0);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
};

namespace Ui {
class MetaEventDock;
}

class MetaEventDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit MetaEventDock(QWidget *parent = 0);
    ~MetaEventDock();

    void disableDragAndDrop();

private slots:
    void setList();
    void activated(const QModelIndex &index);
    void dirSelected(int index);

private:
    void setDirCombo();

private:
    Ui::MetaEventDock *ui;
    MetaEventModel *mModel;
    QTimer mTimer;
};

#endif // METAEVENTDOCK_H
