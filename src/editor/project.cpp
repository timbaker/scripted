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

#include "project.h"

#include "luamanager.h"
#include "node.h"
#include "scriptvariable.h"

Project::Project() :
    mNextID(1),
    mRootNode(new ScriptNode(0, QLatin1String("something")))
{
#if 0
    int i = 0;
    foreach (DraftDefinition *def, CommandsManager::instance()->commands()) {
        DraftObject *object = new DraftObject;
        object->mDefinition = def;
        object->mPosition = QPointF(100 + i++ * 400, 100);
        object->mComment = QLatin1String("Here is a comment regarding this object.");
        mObjects += object;
    }

    DraftConnection *cxn = new DraftConnection;
    cxn->mSender = mObjects[0];
    cxn->mOutput = mObjects[0]->mDefinition->mOutputs[0]->mName;
    cxn->mReceiver = mObjects[1];
    cxn->mInput = mObjects[1]->mDefinition->mInputs[0]->mName;
    mObjects[0]->mConnections += cxn;

    mRootNode->mVariables += new ScriptVariable(QLatin1String("Actor"), QLatin1String("Bob"), QString("some value"));
    mRootNode->mVariables += new ScriptVariable(QLatin1String("Actor"), QLatin1String("Kate"), QString("some value"));
    mRootNode->mVariables += new ScriptVariable(QLatin1String("MapLocation"), QLatin1String("BobKateBedroom"), QString("some value"));
#endif
}

Project::~Project()
{
    delete mRootNode;
}

ScriptVariable *Project::resolveVariable(const QString &name)
{
    return mRootNode->variable(name);
}
