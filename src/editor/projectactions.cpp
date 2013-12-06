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

#include "connectionsdialog.h"
#include "documentmanager.h"
#include "editnodevariabledialog.h"
#include "luadocument.h"
#include "mainwindow.h"
#include "node.h"
#include "nodepropertiesdialog.h"
#include "preferences.h"
#include "preferencesdialog.h"
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
#include <QUndoGroup>

static const QLatin1String KEY_OPEN_PROJECT_DIRECTORY("OpenProjectDirectory");
static const QLatin1String KEY_SAVE_PROJECT_DIRECTORY("SaveProjectDirectory");
static const QLatin1String KEY_OPEN_LUA_DIRECTORY("OpenLuaDirectory");
static const QLatin1String KEY_SAVE_LUA_DIRECTORY("SaveLuaDirectory");

SINGLETON_IMPL(ProjectActions)

ProjectActions::ProjectActions(Ui::MainWindow *actions, QObject *parent) :
    QObject(parent),
    mActions(actions)
{
    mUndoAction = mainwin()->undoGroup()->createUndoAction(this, tr("Undo"));
    mRedoAction = mainwin()->undoGroup()->createRedoAction(this, tr("Redo"));
    mUndoAction->setShortcuts(QKeySequence::Undo);
    mRedoAction->setShortcuts(QKeySequence::Redo);
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));
    undoIcon.addFile(QLatin1String(":images/24x24/edit-undo.png"));
    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    redoIcon.addFile(QLatin1String(":images/24x24/edit-redo.png"));
    mUndoAction->setIcon(undoIcon);
    mRedoAction->setIcon(redoIcon);
//    Tiled::Utils::setThemeIcon(undoAction, "edit-undo");
//    Tiled::Utils::setThemeIcon(redoAction, "edit-redo");
    mActions->menuEdit->insertAction(mActions->menuEdit->actions().at(0), mUndoAction);
    mActions->menuEdit->insertAction(mActions->menuEdit->actions().at(1), mRedoAction);
    mActions->menuEdit->insertSeparator(mActions->menuEdit->actions().at(2));

    mActions->actionNewProject->setShortcut(QKeySequence::New);
    mActions->actionSaveProject->setShortcut(QKeySequence::Save);
    mActions->actionOpenProject->setShortcut(QKeySequence::Open);
//    mActions->actionClose->setShortcut(QKeySequence::Close); // Ctrl+F4
    mActions->actionQuit->setShortcut(QKeySequence::Quit);

    connect(mActions->actionNewProject, SIGNAL(triggered()), SLOT(newProject()));
    connect(mActions->actionOpenProject, SIGNAL(triggered()), SLOT(openProject()));
    connect(mActions->actionNewLuaFile, SIGNAL(triggered()), SLOT(newLuaFile()));
    connect(mActions->actionOpenLuaFile, SIGNAL(triggered()), SLOT(openLuaFile()));
    connect(mActions->actionSaveProject, SIGNAL(triggered()), SLOT(saveFile()));
    connect(mActions->actionSaveProjectAs, SIGNAL(triggered()), SLOT(saveFileAs()));
    connect(mActions->actionRevert, SIGNAL(triggered()), SLOT(revertToSaved()));
    connect(mActions->actionClose, SIGNAL(triggered()), SLOT(close()));
    connect(mActions->actionCloseAll, SIGNAL(triggered()), SLOT(closeAll()));
    connect(mActions->actionQuit, SIGNAL(triggered()), MainWindow::instance(), SLOT(close()));

    connect(mActions->actionPreferences, SIGNAL(triggered()), SLOT(preferencesDialog()));

    connect(mActions->actionEditInputsOutputs, SIGNAL(triggered()), SLOT(sceneScriptDialog()));
    connect(mActions->actionRemoveUnknowns, SIGNAL(triggered()), SLOT(removeUnknowns()));

    mActions->actionAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(mActions->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

Document *ProjectActions::document()
{
    return docman()->currentDocument();
}

ProjectDocument *ProjectActions::projectDoc()
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

void ProjectActions::newLuaFile()
{
    LuaDocument *doc = new LuaDocument(QString());
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

void ProjectActions::openLuaFile()
{
    QSettings settings;

    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = tr("Lua Files (*.lua)");
    filter += selectedFilter;

    QStringList fileNames =
            QFileDialog::getOpenFileNames(MainWindow::instance(), tr("Open Lua File"),
                                          settings.value(KEY_OPEN_LUA_DIRECTORY).toString(),
                                          filter, &selectedFilter);
    if (fileNames.isEmpty())
        return;

    settings.setValue(KEY_OPEN_LUA_DIRECTORY, QFileInfo(fileNames[0]).absolutePath());

    foreach (const QString &fileName, fileNames)
        openLuaFile(fileName);
}

bool ProjectActions::openLuaFile(const QString &fileName)
{
    int n = docman()->findDocument(fileName);
    if (n != -1) {
        Document *doc = docman()->documentAt(n);
        if (doc != docman()->currentDocument())
            docman()->setCurrentDocument(n);
        else
            mainwin()->switchToLuaMode();
        return true;
    }

    docman()->addDocument(new LuaDocument(fileName));
    if (docman()->failedToAdd())
        return false;

    prefs()->addRecentFile(fileName);

    return true;
}

bool ProjectActions::openProject(const QString &fileName)
{
    int n = docman()->findDocument(fileName);
    if (n != -1) {
        Document *doc = docman()->documentAt(n);
        if (doc != docman()->currentDocument())
            docman()->setCurrentDocument(n);
        else
            mainwin()->switchToEditMode();
        return true;
    }

    QFileInfo fileInfo(fileName);
#if 0
    PROGRESS progress(tr("Reading %1").arg(fileInfo.fileName()));
#endif
    ProjectReader reader;
    Project *project = reader.read(fileName);
    if (!project) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Reading Project"),
                              reader.errorString());
        return false;
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
        return false;

    prefs()->addRecentFile(fileName);

    return true;
}

bool ProjectActions::openFile(const QString &fileName)
{
    if (fileName.endsWith(QLatin1String(".lua")))
        return openLuaFile(fileName);
    if (fileName.endsWith(QLatin1String(".pzs")))
        return openProject(fileName);
    return false;
}

bool ProjectActions::saveFile()
{
    QString fileName = document()->fileName();
    if (!fileName.isEmpty())
        return saveFile(fileName);
    return saveFileAs();
}

bool ProjectActions::saveFile(const QString &fileName)
{
    QString error;
    if (!document()->save(fileName, error)) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Saving"), error);
        return false;
    }
    prefs()->addRecentFile(fileName);
    return true;
}

bool ProjectActions::saveFileAs()
{
    QSettings settings;

    QString suggestedFileName;
    if (!document()->fileName().isEmpty()) {
#if 1
        suggestedFileName = document()->fileName();
#else
        const QFileInfo fileInfo(document()->fileName());
        suggestedFileName = fileInfo.path();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += fileInfo.completeBaseName();
        suggestedFileName += QLatin1String(".pzs");
#endif
    } else {
        QString path = settings.value(document()->extension() == QLatin1String(".pzs") ?
                                          KEY_SAVE_PROJECT_DIRECTORY :
                                          KEY_SAVE_LUA_DIRECTORY).toString();
        if (path.isEmpty() || !QDir(path).exists())
            path = prefs()->scriptsDirectory();
        suggestedFileName = path;
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += document()->extension();
    }

    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = document()->filter();
    filter += selectedFilter;

    const QString fileName =
            QFileDialog::getSaveFileName(MainWindow::instance(), QString(), suggestedFileName,
                                         filter, &selectedFilter);
    if (!fileName.isEmpty()) {
        settings.setValue(document()->extension() == QLatin1String(".pzs") ?
                              KEY_SAVE_PROJECT_DIRECTORY : KEY_SAVE_LUA_DIRECTORY, QFileInfo(fileName).absolutePath());
        return saveFile(fileName);
    }
    return false;
}

void ProjectActions::revertToSaved()
{
    if (QMessageBox::question(mainwin(), tr("Revert to Saved"),
                         tr("You will lose your current changes if you continue.\nReally revert %1?")
                         .arg(QDir::toNativeSeparators(document()->fileName()))) == QMessageBox::Yes)
        docman()->currentDocument()->revertToSaved();
}

void ProjectActions::close()
{
    docman()->closeCurrentDocument();
}

void ProjectActions::closeAll()
{
    docman()->closeAllDocuments();
}

void ProjectActions::preferencesDialog()
{
    PreferencesDialog d(mainwin());
    d.exec();
}

void ProjectActions::sceneScriptDialog()
{
    SceneScriptDialog d(projectDoc(), MainWindow::instance());
    d.exec();
}

void ProjectActions::removeConnections(NodeInput *input)
{
    ProjectDocument *doc = projectDoc();
    foreach (BaseNode *node, doc->project()->rootNode()->nodesPlusSelf()) {
        foreach (NodeConnection *cxn, node->connections()) {
            if (cxn->mReceiver == input->node() && cxn->mInput == input->name())
                doc->changer()->doRemoveConnection(node, cxn);
        }
    }
}

void ProjectActions::removeConnections(NodeOutput *output)
{
    ProjectDocument *doc = projectDoc();
    foreach (NodeConnection *cxn, output->node()->connections()) {
        if (cxn->mOutput == output->name())
            doc->changer()->doRemoveConnection(output->node(), cxn);
    }
}

void ProjectActions::removeUnknowns()
{
    ProjectDocument *doc = projectDoc();

    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Remove Unknowns"));
    foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
        foreach (NodeInput *input, node->inputs()) {
            if (!input->isKnown()) {
                removeConnections(input);
                doc->changer()->doRemoveInput(input);
            }
        }
        foreach (NodeOutput *output, node->outputs()) {
            if (!output->isKnown()) {
                removeConnections(output);
                doc->changer()->doRemoveOutput(output);
            }
        }
        foreach (ScriptVariable *var, node->variables()) {
            if (!var->isKnown())
                doc->changer()->doRemoveVariable(var);
        }
    }
    doc->changer()->endUndoMacro();
}

void ProjectActions::removeNode(BaseNode *node)
{
    ProjectDocument *doc = projectDoc();
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
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    doc->changer()->doRenameNode(node, name);
    doc->changer()->endUndoCommand();
}

void ProjectActions::nodePropertiesDialog(BaseNode *node)
{
    NodePropertiesDialog d(projectDoc(), MainWindow::instance());
    d.setNode(node);
    d.exec();
}

void ProjectActions::addInput()
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    int n = 1;
    QString name;
    do {
        name = QString::fromLatin1("Input%1").arg(n++);
    } while (doc->project()->rootNode()->input(name) != 0);
    NodeInput *input = new NodeInput(name);
    doc->changer()->doAddInput(doc->project()->rootNode(),
                               doc->project()->rootNode()->inputCount(),
                               input);
    doc->changer()->endUndoCommand();
}

void ProjectActions::removeInput(int index)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveInput(doc->project()->rootNode()->input(index));
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderInput(int oldIndex, int newIndex)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doReorderInput(doc->project()->rootNode(), oldIndex, newIndex);
    doc->changer()->endUndoCommand();
}

void ProjectActions::changeInput(NodeInput *input, const QString &name, const QString &label)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    NodeInput newValue(input, input->node());
    newValue.setName(name);
    newValue.setLabel(label);
    doc->changer()->doChangeInput(input, &newValue);
    doc->changer()->endUndoCommand();
}

void ProjectActions::addOutput()
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    int n = 1;
    QString name;
    do {
        name = QString::fromLatin1("Output%1").arg(n++);
    } while (doc->project()->rootNode()->output(name) != 0);
    NodeOutput *output = new NodeOutput(name);
    doc->changer()->doAddOutput(doc->project()->rootNode(),
                                doc->project()->rootNode()->outputCount(),
                                output);
    doc->changer()->endUndoCommand();
}

void ProjectActions::removeOutput(int index)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveOutput(doc->project()->rootNode()->output(index));
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderOutput(int oldIndex, int newIndex)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doReorderOutput(doc->project()->rootNode(), oldIndex, newIndex);
    doc->changer()->endUndoCommand();
}

void ProjectActions::changeOutput(NodeOutput *output, const QString &name, const QString &label)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack(), true);
    NodeOutput newValue(output, output->node());
    newValue.setName(name);
    newValue.setLabel(label);
    doc->changer()->doChangeOutput(output, &newValue);
    doc->changer()->endUndoCommand();
}

void ProjectActions::addVariable()
{
    VariablePropertiesDialog d(projectDoc()->project(), NULL, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ScriptVariable *var = new ScriptVariable(d.type(), d.name(), d.label(), QString());
        ProjectDocument *doc = projectDoc();
        doc->changer()->beginUndoCommand(doc->undoStack());
        doc->changer()->doAddVariable(doc->project()->rootNode(),
                                      doc->project()->rootNode()->variableCount(),
                                      var);
        doc->changer()->endUndoCommand();
    }
}

void ProjectActions::removeVariable(ScriptVariable *var)
{
    if (QMessageBox::question(MainWindow::instance(), tr("Remove Variable"),
                              tr("Really remove the variable \"%1\"?").arg(var->name()))
            != QMessageBox::Yes)
        return;

    ProjectDocument *doc = projectDoc();
    Q_ASSERT(doc->project()->rootNode()->variables().contains(var));
    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Remove Variable"));
    foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
        foreach (ScriptVariable *var2, node->variables()) {
            if (var2->variableRef() == var->name()) {
                ScriptVariable newVar2(var2);
                newVar2.setVariableRef(-1, QString());
                doc->changer()->doChangeVariable(var2, &newVar2);
            }
        }
    }
    doc->changer()->doRemoveVariable(var);
    doc->changer()->endUndoMacro();
}

void ProjectActions::variableProperties(ScriptVariable *var)
{
    VariablePropertiesDialog d(projectDoc()->project(), var, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ScriptVariable newVar(var);
        newVar.setName(d.name());
        newVar.setType(d.type());
        newVar.setLabel(d.label());
        ProjectDocument *doc = projectDoc();
        doc->changer()->beginUndoMacro(doc->undoStack(), tr("Change Variable"));
        foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
            foreach (ScriptVariable *var2, node->variables()) {
                if (var2->variableRef() == var->name()) {
                    ScriptVariable newVar2(var2);
                    if (d.name() != var->name())
                        newVar2.setVariableRef(0, d.name());
                    if (d.type() != var->type())
                        newVar2.setVariableRef(-1, QString());
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
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoCommand(doc->undoStack());
    doc->changer()->doRemoveConnection(node, cxn);
    doc->changer()->endUndoCommand();
}

void ProjectActions::reorderConnection(BaseNode *node, int oldIndex, int newIndex)
{
    ProjectDocument *doc = projectDoc();
    doc->changer()->beginUndoMacro(doc->undoStack(), tr("Reorder Connection"));
    doc->changer()->doReorderConnection(node, oldIndex, newIndex);
    doc->changer()->endUndoMacro();
}

void ProjectActions::connectionsDialog(BaseNode *node, const QString &outputName)
{
    ConnectionsDialog d(projectDoc(), node, outputName, mainwin());
    d.exec();
}

void ProjectActions::editNodeVariableValue(ScriptVariable *var)
{
    EditNodeVariableDialog d(var, MainWindow::instance());
    if (d.exec() == QDialog::Accepted) {
        ProjectDocument *doc = projectDoc();
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
    ProjectDocument *pdoc = projectDoc();

    mUndoAction->setVisible(!doc || !doc->isLuaDocument());
    mRedoAction->setVisible(!doc || !doc->isLuaDocument());

    mActions->actionSaveProject->setText(doc ? tr("Save \"%1\"").arg(doc->displayName()) : tr("Save"));
    mActions->actionSaveProject->setEnabled(doc != 0 && doc->isModified());

    mActions->actionSaveProjectAs->setText(doc ? tr("Save \"%1\" As...").arg(doc->displayName()) : tr("Save As..."));
    mActions->actionSaveProjectAs->setEnabled(doc != 0);

    mActions->actionRevert->setText(doc ? tr("Revert \"%1\" to Saved").arg(doc->displayName()) : tr("Revert to Saved"));
    mActions->actionRevert->setEnabled(doc != 0 && doc->fileName().length() && doc->isLuaDocument() && doc->isModified());

    mActions->actionClose->setText(doc ? tr("Close \"%1\"").arg(doc->displayName()) : tr("Close"));
    mActions->actionClose->setEnabled(doc != 0);
    mActions->actionCloseAll->setEnabled(doc != 0);

    mActions->actionEditInputsOutputs->setEnabled(pdoc != 0);
    mActions->actionRemoveUnknowns->setEnabled(pdoc != 0);
}
