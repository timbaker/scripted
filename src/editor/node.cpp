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

#include "node.h"
#include "luamanager.h"
#include "scriptmanager.h"
#include "scriptvariable.h"

bool NodeInput::isKnown() const
{
    return mNode && mNode->isKnown(this);
}

/////

bool NodeOutput::isKnown() const
{
    return mNode && mNode->isKnown(this);
}

/////

void BaseNode::insertInput(int index, NodeInput *input)
{
    Q_ASSERT(input->mNode == NULL);
    input->mNode = this;
    mInputs.insert(index, input);
}

NodeInput *BaseNode::removeInput(int index)
{
    mInputs[index]->mNode = NULL;
    return mInputs.takeAt(index);
}

void BaseNode::insertOutput(int index, NodeOutput *output)
{
    Q_ASSERT(output->mNode == NULL);
    output->mNode = this;
    mOutputs.insert(index, output);
}

NodeOutput *BaseNode::removeOutput(int index)
{
    mOutputs[index]->mNode = NULL;
    return mOutputs.takeAt(index);
}

void BaseNode::insertConnection(int index, NodeConnection *cxn)
{
    Q_ASSERT(cxn->mSender == this);
    Q_ASSERT(!mConnections.contains(cxn));
    mConnections.insert(index, cxn);
}

NodeConnection *BaseNode::removeConnection(NodeConnection *cxn)
{
    Q_ASSERT(cxn->mSender == this);
    int index = mConnections.indexOf(cxn);
    Q_ASSERT(index != -1);
    return mConnections.takeAt(index);
}

ScriptVariable *BaseNode::variable(const QString &name)
{
    foreach (ScriptVariable *p, mVariables)
        if (p->name() == name)
            return p;
    return 0;
}

void BaseNode::insertVariable(int index, ScriptVariable *var)
{
    Q_ASSERT(var->node() == NULL);
    Q_ASSERT(!mVariables.contains(var));
    var->setNode(this);
    mVariables.insert(index, var);
}

ScriptVariable *BaseNode::removeVariable(ScriptVariable *var)
{
    int index = mVariables.indexOf(var);
    Q_ASSERT(index != -1);
    var->setNode(NULL);
    return mVariables.takeAt(index);
}

void BaseNode::initFrom(BaseNode *other)
{
    mName = other->mName;

    qDeleteAll(mVariables);
    mVariables.clear();
    foreach (ScriptVariable *var, other->mVariables)
        insertVariable(variableCount(), new ScriptVariable(var));

    qDeleteAll(mInputs);
    mInputs.clear();
    foreach (NodeInput *input, other->mInputs)
        insertInput(inputCount(), new NodeInput(input));

    qDeleteAll(mOutputs);
    mOutputs.clear();
    foreach (NodeOutput *output, other->mOutputs)
        insertOutput(outputCount(), new NodeOutput(output));
#if 0
    qDeleteALl(mConnections);
    mConnections.clear();
    foreach (NodeConnection *cxn, other->mConnections)
        insertConnection();
#endif
}

/////

LuaNode::LuaNode(int id, const QString &name) :
    BaseNode(id, name),
    mDefinition(0)
{
}

bool LuaNode::isKnown(const ScriptVariable *var)
{
    return mDefinition && mDefinition->node() && mDefinition->node()->variable(var->name());
}

bool LuaNode::isKnown(const NodeInput *input)
{
    return mDefinition && mDefinition->node() && mDefinition->node()->input(input->name());
}

bool LuaNode::isKnown(const NodeOutput *output)
{
    return mDefinition && mDefinition->node() && mDefinition->node()->output(output->name());
}

// FIXME: this is 99% identical to ScriptNode::syncWithScriptInfo()
bool LuaNode::syncWithLuaInfo()
{
    if (!mDefinition || !mDefinition->node()) return false;

    bool changed = false;

    // Sync variables
    QList<ScriptVariable*> variables, myUnknownVariables;
    foreach (ScriptVariable *var, mDefinition->node()->variables()) {
        if (ScriptVariable *myVar = variable(var->name()))
            variables += myVar;
        else {
            variables += new ScriptVariable(var);
            variables.last()->setNode(this);
        }
    }
    foreach (ScriptVariable *myVar, mVariables) {
        if (!variables.contains(myVar))
            myUnknownVariables += myVar;
    }
    if (variables != mVariables) {
        mVariables = variables + myUnknownVariables;
        changed = true;
    }

    // Sync inputs
    QList<NodeInput*> inputs, myUnknownInputs;
    foreach (NodeInput *input, mDefinition->node()->inputs()) {
        if (NodeInput *myInput = this->input(input->name()))
            inputs += myInput;
        else {
            inputs += new NodeInput(input);
            inputs.last()->mNode = this;
        }
    }
    foreach (NodeInput *myInput, mInputs) {
        if (!inputs.contains(myInput))
            myUnknownInputs += myInput;
    }
    if (inputs != mInputs) {
        mInputs = inputs + myUnknownInputs;
        changed = true;
    }

    // Sync outputs
    QList<NodeOutput*> outputs, myUnknownOutputs;
    foreach (NodeOutput *output, mDefinition->node()->outputs()) {
        if (NodeOutput *myOutput = this->output(output->name()))
            outputs += myOutput;
        else {
            outputs += new NodeOutput(output);
            outputs.last()->mNode = this;
        }
    }
    foreach (NodeOutput *myOutput, mOutputs) {
        if (!outputs.contains(myOutput))
            myUnknownOutputs += myOutput;
    }
    if (outputs != mOutputs) {
        mOutputs = outputs + myUnknownOutputs;
        changed = true;
    }

    return changed;
}

void LuaNode::initFrom(LuaNode *other)
{
    BaseNode::initFrom(other);
    mDefinition = other->mDefinition;
}

/////

bool ScriptNode::syncWithScriptInfo()
{
    if (!mInfo || !mInfo->node()) return false;

    bool changed = false;

    // Sync variables
    QList<ScriptVariable*> variables, myUnknownVariables;
    foreach (ScriptVariable *var, mInfo->node()->variables()) {
        if (ScriptVariable *myVar = variable(var->name()))
            variables += myVar;
        else {
            variables += new ScriptVariable(var);
            variables.last()->setNode(this);
        }
    }
    foreach (ScriptVariable *myVar, mVariables) {
        if (!variables.contains(myVar))
            myUnknownVariables += myVar;
    }
    if (variables != mVariables) {
        mVariables = variables + myUnknownVariables;
        changed = true;
    }

    // Sync inputs
    QList<NodeInput*> inputs, myUnknownInputs;
    foreach (NodeInput *input, mInfo->node()->inputs()) {
        if (NodeInput *myInput = this->input(input->name()))
            inputs += myInput;
        else {
            inputs += new NodeInput(input);
            inputs.last()->mNode = this;
        }
    }
    foreach (NodeInput *myInput, mInputs) {
        if (!inputs.contains(myInput))
            myUnknownInputs += myInput;
    }
    if (inputs != mInputs) {
        mInputs = inputs + myUnknownInputs;
        changed = true;
    }

    // Sync outputs
    QList<NodeOutput*> outputs, myUnknownOutputs;
    foreach (NodeOutput *output, mInfo->node()->outputs()) {
        if (NodeOutput *myOutput = this->output(output->name()))
            outputs += myOutput;
        else {
            outputs += new NodeOutput(output);
            outputs.last()->mNode = this;
        }
    }
    foreach (NodeOutput *myOutput, mOutputs) {
        if (!outputs.contains(myOutput))
            myUnknownOutputs += myOutput;
    }
    if (outputs != mOutputs) {
        mOutputs = outputs + myUnknownOutputs;
        changed = true;
    }

    return changed;
}

bool ScriptNode::isKnown(const ScriptVariable *var)
{
    return mInfo && mInfo->node()->variable(var->name());
}

bool ScriptNode::isKnown(const NodeInput *input)
{
    return mInfo && mInfo->node()->input(input->name());
}

bool ScriptNode::isKnown(const NodeOutput *output)
{
    return mInfo && mInfo->node()->output(output->name());
}

void ScriptNode::initFrom(ScriptNode *other)
{
    BaseNode::initFrom(other);
    // don't want the child nodes...
}

