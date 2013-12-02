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

#include "scriptvariable.h"

#include "node.h"

ScriptVariable::ScriptVariable(const QString &type, const QString &name,
                               const QString &value) :
    mNode(0),
    mType(type),
    mName(name),
    mValue(value),
    mVariableRefNodeID(-1)
{
}

ScriptVariable::ScriptVariable(const QString &type, const QString &name,
                               int refNodeID, const QString &refVarName) :
    mNode(0),
    mType(type),
    mName(name),
    mVariableRefNodeID(refNodeID),
    mVariableRef(refVarName)
{
}

ScriptVariable::ScriptVariable(const ScriptVariable *other) :
    mNode(other->mNode),
    mType(other->mType),
    mName(other->mName),
    mValue(other->mValue),
    mVariableRefNodeID(other->mVariableRefNodeID),
    mVariableRef(other->mVariableRef)
{
}

ScriptVariable::ScriptVariable(const ScriptVariable *other, BaseNode *node) :
    mNode(node),
    mType(other->mType),
    mName(other->mName),
    mValue(other->mValue),
    mVariableRefNodeID(other->mVariableRefNodeID),
    mVariableRef(other->mVariableRef)
{

}

bool ScriptVariable::isKnown() const
{
    return mNode && mNode->isKnown(this);
}
