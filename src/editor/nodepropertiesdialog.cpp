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

#include "nodepropertiesdialog.h"
#include "ui_nodepropertiesdialog.h"

#include "luamanager.h"
#include "metaeventmanager.h"
#include "node.h"
#include "projectactions.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "undoredobuttons.h"

#include <QDir>

NodePropertiesDialog::NodePropertiesDialog(ProjectDocument *doc, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodePropertiesDialog),
    mNode(0),
    mCurrentCxn(0),
    mSyncDepth(0)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    UndoRedoButtons *urb = new UndoRedoButtons(doc->undoStack(), this);
    ui->buttonsLayout->insertWidget(0, urb->redoButton());
    ui->buttonsLayout->insertWidget(0, urb->undoButton());

    connect(ui->moveCxnUp, SIGNAL(clicked()), SLOT(moveCxnUp()));
    connect(ui->moveCxnDown, SIGNAL(clicked()), SLOT(moveCxnDown()));
    connect(ui->removeCxn, SIGNAL(clicked()), SLOT(removeCxn()));
    connect(ui->connectionsTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(cxnSelChanged()));
    connect(ui->labelEdit, SIGNAL(textChanged(QString)), SLOT(labelEdited()));

    connect(doc->changer(), SIGNAL(afterRenameNode(BaseNode*,QString)),
            SLOT(afterRenameNode(BaseNode*,QString)));
}

NodePropertiesDialog::~NodePropertiesDialog()
{
    delete ui;
}

void NodePropertiesDialog::setNode(BaseNode *node)
{
    mNode = node;
    mSyncDepth++;
    ui->labelEdit->setText(mNode->label());
    QString type;
    if (node->isEventNode()) type = tr("Event");
    else if (node->isLuaNode()) type = tr("Lua");
    else if (node->isScriptNode()) type = tr("Script");
    else Q_ASSERT(false);
    ui->typeLabel->setText(type);
    ui->IDlabel->setText(QString::number(node->id()));
    if (MetaEventNode *enode = node->asEventNode())
        ui->eventName->setText(enode->eventName());
    else {
        ui->eventNameLabel->hide();
        ui->eventName->hide();
    }
    QString source;
    if (LuaNode *lnode = mNode->asLuaNode())
        source = lnode->source();
    if (MetaEventNode *enode = mNode->asEventNode())
        source = enode->info() ? enode->info()->path() : QString();
    if (ScriptNode *snode = mNode->asScriptNode())
        source = snode->source();
    ui->sourceEdit->setText(QDir::toNativeSeparators(source));
    setConnectionsTable();
    mSyncDepth--;
    syncUI();
}

void NodePropertiesDialog::setConnectionsTable()
{
    NodeConnection *cxn = mCurrentCxn;
    ui->connectionsTable->setNode(mNode);
    if (mNode->connectionCount()) {
        ui->connectionsTable->selectConnection(cxn);
    }

    ui->connectionsTable->header()->resizeSection(0, 250);
}

void NodePropertiesDialog::cxnSelChanged()
{
    mCurrentCxn = 0;
    QModelIndexList selected = ui->connectionsTable->selectionModel()->selectedRows();
    if (selected.size() == 1) {
        int row = selected.first().row();
        mCurrentCxn = mNode->connection(row);
    }

    syncUI();
}

void NodePropertiesDialog::moveCxnUp()
{
    int index = mNode->indexOf(mCurrentCxn);
    ProjectActions::instance()->reorderConnection(mNode, index, index - 1);
}

void NodePropertiesDialog::moveCxnDown()
{
    int index = mNode->indexOf(mCurrentCxn);
    ProjectActions::instance()->reorderConnection(mNode, index, index + 1);
}

void NodePropertiesDialog::removeCxn()
{
    ProjectActions::instance()->removeConnection(mNode, mCurrentCxn);
}

void NodePropertiesDialog::labelEdited()
{
    if (mSyncDepth) return;
    mSyncDepth++;
    ProjectActions::instance()->renameNode(mNode, ui->labelEdit->text());
    mSyncDepth--;
}

void NodePropertiesDialog::afterRenameNode(BaseNode *node, const QString &oldName)
{
    Q_UNUSED(oldName);
    if (mSyncDepth) return;
    mSyncDepth++;
    if (node->label() != ui->labelEdit->text())
        ui->labelEdit->setText(node->label());
    mSyncDepth--;
}

void NodePropertiesDialog::syncUI()
{
    ui->moveCxnUp->setEnabled(mCurrentCxn && (mNode->indexOf(mCurrentCxn) > 0));
    ui->moveCxnDown->setEnabled(mCurrentCxn && (mNode->indexOf(mCurrentCxn) < mNode->connectionCount() - 1));
    ui->removeCxn->setEnabled(mCurrentCxn != 0);
}
