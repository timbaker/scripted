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

#include "luamanager.h"

#include "node.h"
#include "scriptvariable.h"

SINGLETON_IMPL(LuaManager)

LuaManager::LuaManager(QObject *parent) :
    QObject(parent)
{
}

LuaNodeDef *LuaManager::command(const QString &path)
{
    // NPC/AddCharacterToMeta
    // Items/AddInventory

    foreach (LuaNodeDef *def, mCommands) {
        if (def->name() == path)
            return def;
    }
    return 0;
}

bool LuaManager::readLuaFiles()
{
    if (LuaNodeDef *dt = new LuaNodeDef(tr("CheckInventoryItem"))) {
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("Actor"), tr("Entity"), QString("some value")));
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("InventoryItem"), tr("Item"), QString("some value")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input1")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input2")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input3")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output1")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output2")));

        mCommands += dt;
    }

    if (LuaNodeDef *dt = new LuaNodeDef(tr("GiveItem"))) {
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("Actor"), tr("Entity"), QString("some value")));
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("InventoryItem"), tr("Item"), QString("some value")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input1")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input2")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output1")));

        mCommands += dt;
    }

    return true;
}
