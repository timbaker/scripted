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

    ui->tabWidget->setCurrentIndex(0);

    UndoRedoButtons *urb = new UndoRedoButtons(doc->undoStack(), this);
    ui->buttonsLayout->insertWidget(0, urb->redoButton());
    ui->buttonsLayout->insertWidget(0, urb->undoButton());

    connect(ui->addInput, SIGNAL(clicked()), SLOT(addInput()));
    connect(ui->removeInput, SIGNAL(clicked()), SLOT(removeInput()));
    connect(ui->moveInputUp, SIGNAL(clicked()), SLOT(moveInputUp()));
    connect(ui->moveInputDown, SIGNAL(clicked()), SLOT(moveInputDown()));
    connect(ui->inputsList, SIGNAL(itemSelectionChanged()), SLOT(inputSelectionChanged()));
    connect(ui->inputName, SIGNAL(textChanged(QString)), SLOT(inputNameChanged()));

    connect(ui->addOutput, SIGNAL(clicked()), SLOT(addOutput()));
    connect(ui->removeOutput, SIGNAL(clicked()), SLOT(removeOutput()));
    connect(ui->moveOutputUp, SIGNAL(clicked()), SLOT(moveOutputUp()));
    connect(ui->moveOutputDown, SIGNAL(clicked()), SLOT(moveOutputDown()));
    connect(ui->outputsList, SIGNAL(itemSelectionChanged()), SLOT(outputSelectionChanged()));
    connect(ui->outputName, SIGNAL(textChanged(QString)), SLOT(outputNameChanged()));

    connect(mDocument->changer(), SIGNAL(afterAddInput(int,NodeInput*)),
            SLOT(afterAddInput(int,NodeInput*)));
    connect(mDocument->changer(), SIGNAL(afterRemoveInput(int,NodeInput*)),
            SLOT(afterRemoveInput(int,NodeInput*)));
    connect(mDocument->changer(), SIGNAL(afterReorderInput(int,int)),
            SLOT(afterReorderInput(int,int)));
    connect(mDocument->changer(), SIGNAL(afterRenameInput(NodeInput*,QString)),
            SLOT(afterRenameInput(NodeInput*,QString)));

    connect(mDocument->changer(), SIGNAL(afterAddOutput(int,NodeOutput*)),
            SLOT(afterAddOutput(int,NodeOutput*)));
    connect(mDocument->changer(), SIGNAL(afterRemoveOutput(int,NodeOutput*)),
            SLOT(afterRemoveOutput(int,NodeOutput*)));
    connect(mDocument->changer(), SIGNAL(afterReorderOutput(int,int)),
            SLOT(afterReorderOutput(int,int)));
    connect(mDocument->changer(), SIGNAL(afterRenameOutput(NodeOutput*,QString)),
            SLOT(afterRenameOutput(NodeOutput*,QString)));

    foreach (NodeInput *input, mDocument->project()->rootNode()->inputs())
        ui->inputsList->insertItem(ui->inputsList->count(), input->name());

    foreach (NodeOutput *output, mDocument->project()->rootNode()->outputs())
        ui->outputsList->insertItem(ui->outputsList->count(), output->name());

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

void SceneScriptDialog::afterAddOutput(int index, NodeOutput *output)
{
    ui->outputsList->insertItem(index, output->name());
    ui->outputsList->setCurrentRow(index);
}

void SceneScriptDialog::afterRemoveOutput(int index, NodeOutput *output)
{
    delete ui->outputsList->takeItem(index);
}

void SceneScriptDialog::afterReorderOutput(int oldIndex, int newIndex)
{
    QListWidgetItem *item = ui->outputsList->takeItem(oldIndex);
    ui->outputsList->insertItem(newIndex, item);
    ui->outputsList->setCurrentRow(newIndex);
}

void SceneScriptDialog::afterRenameOutput(NodeOutput *output, const QString &oldName)
{
    Q_UNUSED(oldName)
    int row = mDocument->project()->rootNode()->indexOf(output);
    ui->outputsList->item(row)->setText(output->name());

    if (mSelectedOutput != row) return;
    if (mSyncDepth) return;
    mSyncDepth++;
    ui->outputName->setText(output->name());
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
    if (mDocument->project()->isValidInputName(name, mSelectedInput))
        ProjectActions::instance()->renameInput(mSelectedInput, name);
    mSyncDepth--;
}

void SceneScriptDialog::addOutput()
{
    ProjectActions::instance()->addOutput();
}

void SceneScriptDialog::removeOutput()
{
    ProjectActions::instance()->removeOutput(mSelectedOutput);
}

void SceneScriptDialog::moveOutputUp()
{
    ProjectActions::instance()->reorderOutput(mSelectedOutput, mSelectedOutput - 1);
}

void SceneScriptDialog::moveOutputDown()
{
    ProjectActions::instance()->reorderOutput(mSelectedOutput, mSelectedOutput + 1);
}

void SceneScriptDialog::outputSelectionChanged()
{
    mSyncDepth++;
    mSelectedOutput = -1;
    QList<QListWidgetItem*> selected = ui->outputsList->selectedItems();
    if (selected.size()) {
        mSelectedOutput = ui->outputsList->row(selected.first());
        if (NodeOutput *output = mDocument->project()->rootNode()->output(mSelectedOutput))
            ui->outputName->setText(output->name());
    }
    mSyncDepth--;
    syncUI();
}

void SceneScriptDialog::outputNameChanged()
{
    if (mSyncDepth) return;
    mSyncDepth++;
    QString name = ui->outputName->text();
    if (mDocument->project()->isValidOutputName(name, mSelectedOutput))
        ProjectActions::instance()->renameOutput(mSelectedOutput, name);
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

    ui->removeOutput->setEnabled(mSelectedOutput != -1);
    ui->moveOutputUp->setEnabled(mSelectedOutput > 0);
    ui->moveOutputDown->setEnabled((mSelectedOutput != -1) &&
                                  (mSelectedOutput < mDocument->project()->rootNode()->outputCount() - 1));
    if (mSelectedOutput == -1)
        ui->outputName->clear();
    ui->outputName->setEnabled(mSelectedOutput != -1);

    mSyncDepth--;
}
