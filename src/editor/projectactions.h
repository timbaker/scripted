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
    void nodePropertiesDialog(BaseNode *node);

    void addVariable();
    void removeVariable(ScriptVariable *var);
    void variableProperties(ScriptVariable *var);

    void updateActions();

private:
    Ui::MainWindow *mActions;
};

#endif // PROJECTACTIONS_H
