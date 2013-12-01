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

#include "scenescriptdialog.h"
#include "ui_scenescriptdialog.h"

#include "node.h"
#include "project.h"
#include "projectactions.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "undoredobuttons.h"

SceneScriptDialog::SceneScriptDialog(ProjectDocument *doc, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SceneScriptDialog),
    mDocument(doc),
    mSyncDepth(0)
{
    ui->setupUi(this);

    UndoRedoButtons *urb = new UndoRedoButtons(doc->undoStack(), this);
    ui->buttonsLayout->insertWidget(0, urb->redoButton());
    ui->buttonsLayout->insertWidget(0, urb->undoButton());

    connect(ui->addInput, SIGNAL(clicked()), SLOT(addInput()));
    connect(ui->removeInput, SIGNAL(clicked()), SLOT(removeInput()));
    connect(ui->moveInputUp, SIGNAL(clicked()), SLOT(moveInputUp()));
    connect(ui->moveInputDown, SIGNAL(clicked()), SLOT(moveInputDown()));

    connect(ui->inputsList, SIGNAL(itemSelectionChanged()), SLOT(inputSelectionChanged()));

    connect(ui->inputName, SIGNAL(textChanged(QString)), SLOT(inputNameChanged()));

    connect(mDocument->changer(), SIGNAL(afterAddInput(int,NodeInput*)),
            SLOT(afterAddInput(int,NodeInput*)));
    connect(mDocument->changer(), SIGNAL(afterRemoveInput(int,NodeInput*)),
            SLOT(afterRemoveInput(int,NodeInput*)));
    connect(mDocument->changer(), SIGNAL(afterReorderInput(int,int)),
            SLOT(afterReorderInput(int,int)));
    connect(mDocument->changer(), SIGNAL(afterRenameInput(NodeInput*,QString)),
            SLOT(afterRenameInput(NodeInput*,QString)));

    foreach (NodeInput *input, mDocument->project()->rootNode()->inputs())
        ui->inputsList->insertItem(ui->inputsList->count(), input->name());

    syncUI();
}

SceneScriptDialog::~SceneScriptDialog()
{
    delete ui;
}

void SceneScriptDialog::afterAddInput(int index, NodeInput *input)
{
    ui->inputsList->insertItem(index, input->name());
    ui->inputsList->setCurrentRow(index);
}

void SceneScriptDialog::afterRemoveInput(int index, NodeInput *input)
{
    delete ui->inputsList->takeItem(index);
}

void SceneScriptDialog::afterReorderInput(int oldIndex, int newIndex)
{
    QListWidgetItem *item = ui->inputsList->takeItem(oldIndex);
    ui->inputsList->insertItem(newIndex, item);
    ui->inputsList->setCurrentRow(newIndex);
}

void SceneScriptDialog::afterRenameInput(NodeInput *input, const QString &oldName)
{
    Q_UNUSED(oldName)
    int row = mDocument->project()->rootNode()->indexOf(input);
    ui->inputsList->item(row)->setText(input->name());

    if (mSelectedInput != row) return;
    if (mSyncDepth) return;
    mSyncDepth++;
    ui->inputName->setText(input->name());
    mSyncDepth--;
}

void SceneScriptDialog::addInput()
{
    ProjectActions::instance()->addInput();
}

void SceneScriptDialog::removeInput()
{
    ProjectActions::instance()->removeInput(mSelectedInput);
}

void SceneScriptDialog::moveInputUp()
{
    ProjectActions::instance()->reorderInput(mSelectedInput, mSelectedInput - 1);
}

void SceneScriptDialog::moveInputDown()
{
    ProjectActions::instance()->reorderInput(mSelectedInput, mSelectedInput + 1);
}

void SceneScriptDialog::inputSelectionChanged()
{
    mSyncDepth++;
    mSelectedInput = -1;
    QList<QListWidgetItem*> selected = ui->inputsList->selectedItems();
    if (selected.size()) {
        mSelectedInput = ui->inputsList->row(selected.first());
        if (NodeInput *input = mDocument->project()->rootNode()->input(mSelectedInput))
            ui->inputName->setText(input->name());
    }
    mSyncDepth--;
    syncUI();
}

void SceneScriptDialog::inputNameChanged()
{
    if (mSyncDepth) return;
    mSyncDepth++;
    QString name = ui->inputName->text();
    if (mDocument->project()->isValidInputName(name))
        ProjectActions::instance()->renameInput(mSelectedInput, name);
    mSyncDepth--;
}

void SceneScriptDialog::syncUI()
{
    mSyncDepth++;
    ui->removeInput->setEnabled(mSelectedInput != -1);
    ui->moveInputUp->setEnabled(mSelectedInput > 0);
    ui->moveInputDown->setEnabled((mSelectedInput != -1) &&
                                  (mSelectedInput < mDocument->project()->rootNode()->inputCount() - 1));
    if (mSelectedInput == -1)
        ui->inputName->clear();
    ui->inputName->setEnabled(mSelectedInput != -1);
    mSyncDepth--;
}
