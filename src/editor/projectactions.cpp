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

#include "projectactions.h"

#include "documentmanager.h"
#include "editnodevariabledialog.h"
#include "node.h"
#include "mainwindow.h"
#include "nodepropertiesdialog.h"
#include "preferences.h"
#include "progress.h"
#include "project.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "projectreader.h"
#include "scenescriptdialog.h"
#include "variablepropertiesdialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

static const QLatin1String KEY_OPEN_PROJECT_DIRECTORY("OpenProjectDirectory");
static const QLatin1String KEY_SAVE_PROJECT_DIRECTORY("SaveProjectDirectory");

SINGLETON_IMPL(ProjectActions)

ProjectActions::ProjectActions(Ui::MainWindow *actions, QObject *parent) :
    QObject(parent),
    mActions(actions)
{
    connect(mActions->actionNewProject, SIGNAL(triggered()), SLOT(newProject()));
    connect(mActions->actionOpenProject, SIGNAL(triggered()), SLOT(openProject()));
    connect(mActions->actionSaveProject, SIGNAL(triggered()), SLOT(saveProject()));
    connect(mActions->actionSaveProjectAs, SIGNAL(triggered()), SLOT(saveProjectAs()));
    connect(mActions->actionQuit, SIGNAL(triggered()), MainWindow::instance(), SLOT(close()));

    connect(mActions->actionPreferences, SIGNAL(triggered()), SLOT(preferencesDialog()));

    connect(mActions->actionEditInputsOutputs, SIGNAL(triggered()), SLOT(sceneScriptDialog()));
}

ProjectDocument *ProjectActions::document()
{
    if (Document *doc = docman()->currentDocument())
        return doc->asProjectDocument();
    return 0;
}

void ProjectActions::newProject()
{
    Project *prj = new Project();
    ProjectDocument *doc = new ProjectDocument(prj, QString());
    docman()->addDocument(doc);
}

void ProjectActions::openProject()
{
    QSettings settings;

    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = tr("PZDraft project (*.pzs)");
    filter += selectedFilter;

    QStringList fileNames =
            QFileDialog::getOpenFileNames(MainWindow::instance(), tr("Open Project"),
                                          settings.value(KEY_OPEN_PROJECT_DIRECTORY).toString(),
                                          filter, &selectedFilter);
    if (fileNames.isEmpty())
        return;

    settings.setValue(KEY_OPEN_PROJECT_DIRECTORY, QFileInfo(fileNames[0]).absolutePath());

    foreach (const QString &fileName, fileNames)
        openProject(fileName);
}

void ProjectActions::openProject(const QString &fileName)
{
    int n = docman()->findDocument(fileName);
    if (n != -1) {
        docman()->setCurrentDocument(n);
//        MainWindow::instance()->switchToEditMode();
        return;
    }

    QFileInfo fileInfo(fileName);
    PROGRESS progress(tr("Reading %1").arg(fileInfo.fileName()));

    ProjectReader reader;
    Project *project = reader.read(fileName);
    if (!project) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Reading Project"),
                              reader.errorString());
        return;
    }

#if 0
    foreach (BaseNode *node, project->rootNode()->nodes()) {
        if (!node->mDefinition)
            continue;
        foreach (DraftProperty *p, node->mDefinition->mProperties) {
            if (node->variable(p->mName) == NULL) {
                DraftProperty *np = new DraftProperty(p);
                node->insertVariable(node->variableCount(), np);
            }
        }
    }
#endif

    docman()->addDocument(new ProjectDocument(project, fileName));
    if (docman()->failedToAdd())
        return;

    prefs()->addRecentFile(fileName);
}

bool ProjectActions::saveProject()
{
    QString fileName = document()->fileName();
    if (!fileName.isEmpty())
        return saveProject(fileName);
    return saveProjectAs();
}

bool ProjectActions::saveProject(const QString &fileName)
{
    QString error;
    if (!document()->save(fileName, error)) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Saving Project"), error);
        return false;
    }
    prefs()->addRecentFile(fileName);
    return true;
}

bool ProjectActions::saveProjectAs()
{
    QSettings settings;

    QString suggestedFileName;
    if (!document()->fileName().isEmpty()) {
        const QFileInfo fileInfo(document()->fileName());
        suggestedFileName = fileInfo.path();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += fileInfo.completeBaseName();
        suggestedFileName += QLatin1String(".pzs");
    } else {
        QString path = settings.value(KEY_SAVE_PROJECT_DIRECTORY).toString();
        if (path.isEmpty() || !QDir(path).exists())
            path = QDir::currentPath();
        suggestedFileName = path;
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += tr("untitled.pzs");
    }

    const QString fileName =
            QFileDialog::getSaveFileName(MainWindow::instance(), QString(), suggestedFileName,
                                         tr("PZDraft project (*.pzs)"));
    if (!fileName.isEmpty()) {
        settings.setValue(KEY_SAVE_PROJECT_DIRECTORY, QFileInfo(fileName).absolutePath());
        return saveProject(fileName);
    }
    return false;
}

void ProjectActions::preferencesDialog()
{

}

void ProjectActions::sceneScriptDialog()
{
    SceneScriptDialog d(document(), MainWindow::instance());
    d.exec();
}

void ProjectActions::removeNode(BaseNode *node)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Remove Node"));
    foreach (BaseNode *node2, doc->project()->rootNode()->nodes()) {
        for (int i = 0; i < node2->connectionCount(); i++) {
            NodeConnection *cxn = node2->connection(i);
            if (cxn->mSender == node || cxn->mReceiver == node) {
                doc->changer()->doRemoveConnection(node2, cxn);
                --i;
            }
        }
    }
    doc->changer()->doRemoveNode(node);
    doc->changer()->endUndoMacro();
}

void ProjectActions::renameNode(BaseNode *node, const QString &name)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    doc->changer()->doRenameNode(node, name);
    doc->changer()->endUndoCommand();
}

void ProjectActions::nodePropertiesDialog(BaseNode *node)
{
    NodePropertiesDialog d(document(), MainWindow::instance());
    d.setNode(node);
    d.exec();
}

void ProjectActions::addInput()
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    int n = 1;
    QString name;
    do {
        name = QString::fromLatin1("Input%1").arg(n++);
    } while (doc->project()->rootNode()->input(name) != 0);
    NodeInput *input = new NodeInput(name);
    doc->changer()->doAddInput(doc->project()->rootNode()->inputCount(), input);
    doc->changer()->endUndoCommand();
}

void ProjectActions::removeInput(int index)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveInput(doc->project()->rootNode()->input(index));
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderInput(int oldIndex, int newIndex)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doReorderInput(oldIndex, newIndex);
    doc->changer()->endUndoCommand();
}

void ProjectActions::renameInput(int index, const QString &name)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    doc->changer()->doRenameInput(doc->project()->rootNode()->input(index), name);
    doc->changer()->endUndoCommand();
}

void ProjectActions::addOutput()
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    int n = 1;
    QString name;
    do {
        name = QString::fromLatin1("Output%1").arg(n++);
    } while (doc->project()->rootNode()->output(name) != 0);
    NodeOutput *output = new NodeOutput(name);
    doc->changer()->doAddOutput(doc->project()->rootNode()->outputCount(), output);
    doc->changer()->endUndoCommand();
}

void ProjectActions::removeOutput(int index)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveOutput(doc->project()->rootNode()->output(index));
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderOutput(int oldIndex, int newIndex)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doReorderOutput(oldIndex, newIndex);
    doc->changer()->endUndoCommand();
}

void ProjectActions::renameOutput(int index, const QString &name)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    doc->changer()->doRenameOutput(doc->project()->rootNode()->output(index), name);
    doc->changer()->endUndoCommand();
}

void ProjectActions::addVariable()
{
    VariablePropertiesDialog d(NULL, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ScriptVariable *var = new ScriptVariable(d.type(), d.name(), QString(), QString());
        ProjectDocument *doc = document();
        doc->changer()->beginUndoCommand(doc->undoStack());
        doc->changer()->doAddVariable(doc->project()->rootNode()->variableCount(), var);
        doc->changer()->endUndoCommand();
    }
}

void ProjectActions::removeVariable(ScriptVariable *var)
{
    if (QMessageBox::question(MainWindow::instance(), tr("Remove Variable"),
                              tr("Really remove the variable \"%1\"?").arg(var->name()))
            != QMessageBox::Yes)
        return;

    ProjectDocument *doc = document();
    Q_ASSERT(doc->project()->rootNode()->variables().contains(var));
    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Remove Variable"));
    foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
        foreach (ScriptVariable *var2, node->variables()) {
            if (var2->variableRef() == var->name()) {
                ScriptVariable newVar2(var2);
                newVar2.setVariableRef(QString());
                doc->changer()->doChangeVariable(var2, &newVar2);
            }
        }
    }
    doc->changer()->doRemoveVariable(var);
    doc->changer()->endUndoMacro();
}

void ProjectActions::variableProperties(ScriptVariable *var)
{
    VariablePropertiesDialog d(var, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ScriptVariable newVar(var);
        newVar.setName(d.name());
        newVar.setType(d.type());
        ProjectDocument *doc = document();
        doc->changer()->beginUndoMacro(doc->undoStack(), tr("Change Variable"));
        foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
            foreach (ScriptVariable *var2, node->variables()) {
                if (var2->variableRef() == var->name()) {
                    ScriptVariable newVar2(var2);
                    if (d.name() != var->name())
                        newVar2.setVariableRef(d.name());
                    if (d.type() != var->type())
                        newVar2.setVariableRef(QString());
                    doc->changer()->doChangeVariable(var2, &newVar2);
                }
            }
        }
        doc->changer()->doChangeVariable(var, &newVar);
        doc->changer()->endUndoMacro();
    }
}

void ProjectActions::removeConnection(BaseNode *node, NodeConnection *cxn)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveConnection(node, cxn);
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderConnection(BaseNode *node, int oldIndex, int newIndex)
{
    ProjectDocument *doc = document();
    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Reorder Connection"));
    doc->changer()->doReorderConnection(node, oldIndex, newIndex);
    doc->changer()->endUndoMacro();
}

void ProjectActions::editNodeVariableValue(ScriptVariable *var)
{
    EditNodeVariableDialog d(var, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ProjectDocument *doc = document();
        doc->changer()->beginUndoCommand(doc->undoStack());
        ScriptVariable newVar(var);
        newVar.setValue(d.value());
        doc->changer()->doChangeVariable(var, &newVar);
        doc->changer()->endUndoCommand();
    }
}

void ProjectActions::updateActions()
{
    Document *doc = docman()->currentDocument();
    ProjectDocument *pdoc = document();
    mActions->actionSaveProject->setEnabled(doc != 0 && doc->isModified());
    mActions->actionSaveProjectAs->setEnabled(doc != 0);
    mActions->actionEditInputsOutputs->setEnabled(pdoc != 0);
}
