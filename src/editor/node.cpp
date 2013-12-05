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
#include "metaeventmanager.h"
#include "scriptmanager.h"
#include "scriptvariable.h"

bool NodeInput::isKnown() const
{
    return mNode && mNode->isKnown(this);
}

bool NodeInput::hasBadConnections()
{
    if (mNode) {
        foreach (NodeConnection *cxn, mNode->connections()) {
            // Only root node inputs have connections
            if ((cxn->mOutput == mName) && !cxn->mReceiver->input(cxn->mInput))
                return true;
        }
    }
    return false;
}

/////

bool NodeOutput::isKnown() const
{
    return mNode && mNode->isKnown(this);
}

bool NodeOutput::hasBadConnections()
{
    if (mNode) {
        foreach (NodeConnection *cxn, mNode->connections()) {
            if (cxn->mOutput != mName)
                continue;
            if (cxn->mReceiver->isProjectRootNode()) {
                if (!cxn->mReceiver->output(cxn->mInput))
                    return true;
            } else {
                if (!cxn->mReceiver->input(cxn->mInput))
                    return true;
            }
        }
    }
    return false;
}

/////

BaseNode::~BaseNode()
{
    qDeleteAll(mInputs);
    qDeleteAll(mOutputs);
    qDeleteAll(mVariables);
}

void BaseNode::insertInput(int index, NodeInput *input)
{
    Q_ASSERT(input->node() == NULL);
    input->setNode(this);
    mInputs.insert(index, input);
}

NodeInput *BaseNode::removeInput(int index)
{
    mInputs[index]->setNode(NULL);
    return mInputs.takeAt(index);
}

void BaseNode::insertOutput(int index, NodeOutput *output)
{
    Q_ASSERT(output->node() == NULL);
    output->setNode(this);
    mOutputs.insert(index, output);
}

NodeOutput *BaseNode::removeOutput(int index)
{
    mOutputs[index]->setNode(NULL);
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

void BaseNode::initFrom(const BaseNode *other)
{
    mLabel = other->mLabel;

    qDeleteAll(mVariables);
    mVariables.clear();
    foreach (ScriptVariable *var, other->mVariables)
        insertVariable(variableCount(), new ScriptVariable(var, 0));

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

bool BaseNode::syncWithInfo(BaseNode *infoNode)
{
    if (!infoNode) return false;

    bool changed = false;

    // Sync variables
    QList<ScriptVariable*> variables, myUnknownVariables;
    foreach (ScriptVariable *var, infoNode->variables()) {
        if (ScriptVariable *myVar = variable(var->name())) {
            if (myVar->label() != var->label()) {
                myVar->setLabel(var->label());
                changed = true;
            }
            variables += myVar;
        } else {
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
    foreach (NodeInput *input, infoNode->inputs()) {
        if (NodeInput *myInput = this->input(input->name())) {
            if (myInput->label() != input->label()) {
                myInput->setLabel(input->label());
                changed = true;
            }
            inputs += myInput;
        } else {
            inputs += new NodeInput(input);
            inputs.last()->setNode(this);
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
    foreach (NodeOutput *output, infoNode->outputs()) {
        if (NodeOutput *myOutput = this->output(output->name())) {
            if (myOutput->label() != output->label()) {
                myOutput->setLabel(output->label());
                changed = true;
            }
            outputs += myOutput;
        } else {
            outputs += new NodeOutput(output);
            outputs.last()->setNode(this);
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

/////


MetaEventNode::MetaEventNode(int id, const QString &name) :
    BaseNode(id, name),
    mInfo(0)
{
    insertOutput(0, new NodeOutput(QLatin1String("Output")));
}

MetaEventNode::MetaEventNode(int id, const MetaEventNode &other) :
    BaseNode(id, other.mLabel)
{
    initFrom(&other);
}

bool MetaEventNode::isKnown(const ScriptVariable *var)
{
    return mInfo && mInfo->node() && mInfo->node()->variable(var->name());
}

bool MetaEventNode::isKnown(const NodeInput *input)
{
    return false;
}

bool MetaEventNode::isKnown(const NodeOutput *output)
{
    return mOutputs.contains(const_cast<NodeOutput*>(output)); // our single output
}

bool MetaEventNode::syncWithInfo()
{
    if (!mInfo || !mInfo->node()) return false;
    return BaseNode::syncWithInfo(mInfo->node());
}

void MetaEventNode::initFrom(const MetaEventNode *other)
{
    BaseNode::initFrom(other);
    mInfo = other->mInfo;
}

/////

LuaNode::LuaNode(int id, const QString &name) :
    BaseNode(id, name),
    mInfo(0)
{
}

LuaNode::LuaNode(int id, const LuaNode &other) :
    BaseNode(id, other.label())
{
    initFrom(&other);
}

bool LuaNode::isKnown(const ScriptVariable *var)
{
    return mInfo && mInfo->node() && mInfo->node()->variable(var->name());
}

bool LuaNode::isKnown(const NodeInput *input)
{
    return mInfo && mInfo->node() && mInfo->node()->input(input->name());
}

bool LuaNode::isKnown(const NodeOutput *output)
{
    return mInfo && mInfo->node() && mInfo->node()->output(output->name());
}

// FIXME: this is 99% identical to ScriptNode::syncWithInfo()
bool LuaNode::syncWithInfo()
{
    if (!mInfo || !mInfo->node()) return false;
#if 1
    return syncWithInfo(mInfo->node());
#else
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
#endif
}

void LuaNode::initFrom(const LuaNode *other)
{
    BaseNode::initFrom(other);
    mInfo = other->mInfo;
}

/////

ScriptNode::~ScriptNode()
{
    qDeleteAll(mNodes);
}

bool ScriptNode::syncWithInfo()
{
    if (!mInfo || !mInfo->node()) return false;
#if 1
    return syncWithInfo(mInfo->node());
#else
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
#endif
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
