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

#ifndef PROJECTCHANGER_H
#define PROJECTCHANGER_H

#include "editor_global.h"

#include <QList>
#include <QObject>
#include <QPointF>
#include <QUndoCommand>

class QColor;
class QImage;
class QRect;
class QUndoStack;

class ProjectChange
{
public:
    ProjectChange(ProjectChanger *changer);
    virtual ~ProjectChange();

    void setChanger(ProjectChanger *changer);

    virtual void redo() = 0;
    virtual void undo() = 0;
    virtual bool merge(ProjectChange *other) { Q_UNUSED(other); return false; }

    // UndoCommand::id()
    virtual int id() const { return -1; }

    virtual QString text() const = 0;

protected:
    ProjectChanger *mChanger;
};

class ProjectChangeUndoCommand: public QUndoCommand
{
public:
    ProjectChangeUndoCommand(ProjectChange *change, bool mergeable);
    ~ProjectChangeUndoCommand();

    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *other);

    int id() const;

private:
    ProjectChange *mChange;
    bool mMergeable;
};

class ProjectChanger : public QObject
{
    Q_OBJECT
public:
    ProjectChanger(Project *project);
    ~ProjectChanger();

    Project *project() const
    { return mProject; }

    const ProjectChangeList &changes() const
    { return mChanges; }

    int changeCount() const
    { return mChanges.size(); }

    ProjectChangeList takeChanges(bool undoFirst = true);
    void undoAndForget();

    void beginUndoMacro(QUndoStack *undoStack, const QString &text);
    void endUndoMacro();

    void beginUndoCommand(QUndoStack *undoStack, bool mergeable = false);
    void endUndoCommand();

    void undo();

    /////
    void doAddNode(int index, BaseNode *node);
    void doRemoveNode(BaseNode *node);
    void doMoveNode(BaseNode *node, const QPointF &pos);
    void doRenameNode(BaseNode *node, const QString &name);

    void doAddInput(int index, NodeInput *input);
    void doRemoveInput(NodeInput *input);
    void doReorderInput(int oldIndex, int newIndex);
    void doRenameInput(NodeInput *input, const QString &name);

    void doAddOutput(int index, NodeOutput *output);
    void doRemoveOutput(NodeOutput *output);
    void doReorderOutput(int oldIndex, int newIndex);

    void doAddConnection(int index, NodeConnection *cxn);
    void doRemoveConnection(BaseNode *node, NodeConnection *cxn);
    void doReorderConnection(BaseNode *node, int oldIndex, int newIndex);

    void doAddVariable(int index, ScriptVariable *var);
    void doRemoveVariable(ScriptVariable *var);
    void doChangeVariable(ScriptVariable *var, const ScriptVariable *newVar);
    void doSetVariableValue(ScriptVariable *var, const QString &value);
    void doSetVariableRef(ScriptVariable *var, const QString &varName);
    /////

signals:
    void afterAddNode(int index, BaseNode *node);
    void beforeRemoveNode(int index, BaseNode *node);
    void afterRemoveNode(int index, BaseNode *node);
    void afterRenameNode(BaseNode *node, const QString &oldName);
    void afterMoveNode(BaseNode *object, const QPointF &oldPos);

    void afterAddInput(int index, NodeInput *input);
    void afterRemoveInput(int index, NodeInput *input);
    void afterReorderInput(int oldIndex, int newIndex);
    void afterRenameInput(NodeInput *input, const QString &oldName);

    void afterAddOutput(int index, NodeOutput *output);
    void afterRemoveOutput(int index, NodeOutput *output);
    void afterReorderOutput(int oldIndex, int newIndex);
    void afterRenameOutput(NodeOutput *output, const QString &name);

    void afterAddConnection(int index, NodeConnection *cxn);
    void beforeRemoveConnection(int index, NodeConnection *cxn);
    void afterRemoveConnection(int index, NodeConnection *cxn);
    void afterReorderConnection(BaseNode *node, int oldIndex, int newIndex);

    void afterAddVariable(int index, ScriptVariable *var);
    void beforeRemoveVariable(int index, ScriptVariable *var);
    void afterRemoveVariable(int index, ScriptVariable *var);
    void afterChangeVariable(ScriptVariable *var, const ScriptVariable *oldValue);

private:
    void addChange(ProjectChange *change);

private:
    Project *mProject;
    ProjectChangeList mChanges;
    ProjectChangeList mChangesReversed;
    QUndoStack *mUndoStack;
#ifndef QT_NO_DEBUG
    int mUndoMacroDepth;
    int mUndoCommandDepth;
#endif
    bool mUndoMergeable;
};

#endif // PROJECTCHANGER_H
