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

#ifndef PROJECTACTIONS_H
#define PROJECTACTIONS_H

#include "editor_global.h"
#include "singleton.h"
#include "ui_mainwindow.h"

#include <QObject>

class ProjectActions : public QObject, public Singleton<ProjectActions>
{
    Q_OBJECT
public:
    explicit ProjectActions(Ui::MainWindow *actions, QObject *parent = 0);

    Ui::MainWindow *actions()
    { return mActions; }

    ProjectDocument *document();

public slots:
    void newProject();
    void openProject();
    void openProject(const QString &fileName);
    bool saveProject();
    bool saveProject(const QString &fileName);
    bool saveProjectAs();

    void preferencesDialog();

    void sceneScriptDialog();
    void removeConnections(NodeInput *input);
    void removeConnections(NodeOutput *output);
    void removeUnknowns();

    void removeNode(BaseNode *node);
    void renameNode(BaseNode *node, const QString &name);
    void nodePropertiesDialog(BaseNode *node);

    void addInput();
    void removeInput(int index);
    void reorderInput(int oldIndex, int newIndex);
    void changeInput(NodeInput *input, const QString &name, const QString &label);

    void addOutput();
    void removeOutput(int index);
    void reorderOutput(int oldIndex, int newIndex);
    void changeOutput(NodeOutput *output, const QString &name, const QString &label);

    void addVariable();
    void removeVariable(ScriptVariable *var);
    void variableProperties(ScriptVariable *var);

    void removeConnection(BaseNode *node, NodeConnection *cxn);
    void reorderConnection(BaseNode *node, int oldIndex, int newIndex);

    void connectionsDialog(BaseNode *node, const QString &outputName);

    void editNodeVariableValue(ScriptVariable *var);

    void updateActions();

private:
    Ui::MainWindow *mActions;
};

#endif // PROJECTACTIONS_H
