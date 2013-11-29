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
#include "scriptvariable.h"

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

void BaseNode::insertVariable(int index, ScriptVariable *p)
{
    Q_ASSERT(!mVariables.contains(p));
    mVariables.insert(index, p);
}

ScriptVariable *BaseNode::removeVariable(ScriptVariable *p)
{
    int index = mVariables.indexOf(p);
    Q_ASSERT(index != -1);
    return mVariables.takeAt(index);
}
