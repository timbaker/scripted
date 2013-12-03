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

#ifndef SCENESCRIPTDIALOG_H
#define SCENESCRIPTDIALOG_H

#include "editor_global.h"
#include <QDialog>

namespace Ui {
class SceneScriptDialog;
}

class SceneScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SceneScriptDialog(ProjectDocument *doc, QWidget *parent = 0);
    ~SceneScriptDialog();

private slots:
    void afterAddInput(BaseNode *node, int index, NodeInput *input);
    void afterRemoveInput(BaseNode *node, int index, NodeInput *input);
    void afterReorderInput(BaseNode *node, int oldIndex, int newIndex);
    void afterRenameInput(NodeInput *input, const QString &oldName);

    void afterAddOutput(BaseNode *node, int index, NodeOutput *output);
    void afterRemoveOutput(BaseNode *node, int index, NodeOutput *output);
    void afterReorderOutput(BaseNode *node, int oldIndex, int newIndex);
    void afterRenameOutput(NodeOutput *output, const QString &oldName);

    void addInput();
    void removeInput();
    void moveInputUp();
    void moveInputDown();
    void inputSelectionChanged();
    void inputNameChanged();

    void addOutput();
    void removeOutput();
    void moveOutputUp();
    void moveOutputDown();
    void outputSelectionChanged();
    void outputNameChanged();

    void syncUI();

private:
    Ui::SceneScriptDialog *ui;
    ProjectDocument *mDocument;
    int mSelectedInput;
    int mSelectedOutput;
    int mSyncDepth;
};

#endif // SCENESCRIPTDIALOG_H
