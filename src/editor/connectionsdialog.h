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

#ifndef CONNECTIONSDIALOG_H
#define CONNECTIONSDIALOG_H

#include "editor_global.h"
#include <QDialog>

namespace Ui {
class ConnectionsDialog;
}

class ConnectionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsDialog(ProjectDocument *doc, BaseNode *node,
                               const QString &outputName, QWidget *parent = 0);
    ~ConnectionsDialog();

private slots:
    void selectionChanged();
    void moveUp();
    void moveDown();
    void remove();
    void syncUI();

private:
    void setConnectionsTable();
    NodeConnection *prev();
    NodeConnection *next();

private:
    Ui::ConnectionsDialog *ui;
    BaseNode *mNode;
    QString mOutputName;
    NodeConnection *mCurrentCxn;
};

#endif // CONNECTIONSDIALOG_H
