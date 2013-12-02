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

#include <QDir>
#include <QFileInfo>

SINGLETON_IMPL(LuaManager)

LuaManager::LuaManager(QObject *parent) :
    QObject(parent)
{
    connect(&mFileSystemWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));

    mChangedFilesTimer.setInterval(500);
    mChangedFilesTimer.setSingleShot(true);
    connect(&mChangedFilesTimer, SIGNAL(timeout()),
            SLOT(fileChangedTimeout()));
}

LuaInfo *LuaManager::luaInfo(const QString &fileName, const QString &relativeTo)
{
    QString path = canonicalPath(fileName, relativeTo);
    if (path.isEmpty())
        return 0;

    if (mLuaInfo.contains(path))
        return mLuaInfo[path];

    LuaNode *node = loadLua(path);
    if (!node)
        return 0;

    LuaInfo *info = new LuaInfo;
    info->mPath = path;
    info->mNode = node;
    mLuaInfo[path] = info;
    mFileSystemWatcher.addPath(path);

    return info;
}

QString LuaManager::canonicalPath(const QString &fileName, const QString &relativeTo)
{
    QString path = fileName;

    if (QDir::isRelativePath(fileName)) {
        Q_ASSERT(!relativeTo.isEmpty());
        Q_ASSERT(!QDir::isRelativePath(relativeTo));
        path = relativeTo + QLatin1Char('/') + fileName;
    }

    if (!fileName.endsWith(QLatin1String(".lua")))
        path += QLatin1String(".lua");

    QFileInfo info(path);
    if (info.exists())
        return info.canonicalFilePath();

    return QString();
}

#include <QApplication>

bool LuaManager::readLuaFiles()
{
    QDir dir(qApp->applicationDirPath() + "/../../../../scripts");
    foreach (QFileInfo fileInfo, dir.entryInfoList(QStringList() << QLatin1String("*.lua"))) {
        if (LuaInfo *info = luaInfo(fileInfo.absoluteFilePath()))
            mCommands += info;
    }

#if 0
    if (LuaInfo *dt = new LuaInfo(tr("CheckInventoryItem"))) {
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("Actor"), tr("Entity"), QString("some value")));
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("InventoryItem"), tr("Item"), QString("some value")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input1")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input2")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input3")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output1")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output2")));

        mCommands += dt;
    }

    if (LuaInfo *dt = new LuaInfo(tr("GiveItem"))) {
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("Actor"), tr("Entity"), QString("some value")));
        dt->insertVariable(dt->variableCount(), new ScriptVariable(QLatin1String("InventoryItem"), tr("Item"), QString("some value")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input1")));
        dt->insertInput(dt->inputCount(), new NodeInput(tr("Input2")));
        dt->insertOutput(dt->outputCount(), new NodeOutput(tr("Output1")));

        mCommands += dt;
    }
#endif

    return true;
}

void LuaManager::fileChanged(const QString &path)
{
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();
}

void LuaManager::fileChangedTimeout()
{
    foreach (const QString &path, mChangedFiles) {
        if (mLuaInfo.contains(path)) {
            noise() << "LuaManager::fileChanged" << path;
            mFileSystemWatcher.removePath(path);
            QFileInfo info(path);
            if (info.exists()) {
                mFileSystemWatcher.addPath(path);
                LuaInfo *info = mLuaInfo[path];
                delete info->mNode;
                info->mNode = loadLua(path);
                if (!info->mNode) {
                    // ???
                }
                emit luaChanged(info);
            }
        }
    }

    mChangedFiles.clear();
}

extern "C" {

#include "lualib.h"
#include "lauxlib.h"

// see luaconf.h
// these are where print() calls go
void luai_writestring(const char *s, int len)
{
}

void luai_writeline()
{
}

} // extern "C"

LuaNode *LuaManager::loadLua(const QString &fileName)
{
    if (!QFileInfo(fileName).exists())
        return NULL;

    if (lua_State *L = luaL_newstate()) {

        LuaNode *ret = new LuaNode(0, QFileInfo(fileName).baseName());

        luaL_openlibs(L);
        lua_pushboolean(L, true);
        lua_setglobal(L, "_SCRIPTED_");
        int status = luaL_loadfile(L, fileName.toLatin1().data());
        if (status == LUA_OK) {
            status = lua_pcall(L, 0, 0, -1); // call the closure?
            if (status == LUA_OK) {
                lua_getglobal(L, "editor");
                int tblidx = lua_gettop(L);
                if (lua_istable(L, tblidx)) {
                    lua_pushnil(L); // push space on stack for the key
                    while (lua_next(L, tblidx) != 0) { // pop a key, push key then value
                        const char *key = lua_isstring(L, -2) ? lua_tostring(L, -2) : "";
                        if (!strcmp(key, "inputs")) {
                            if (!lua_istable(L, -1))
                                return NULL; // FIXME: lua_close
                            lua_pushnil(L); // space for key
                            while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                const char *s = lua_tostring(L, -1);
                                qDebug() << "input" << s;
                                ret->insertInput(ret->inputCount(), new NodeInput(QLatin1String(s)));
                                lua_pop(L, 1); // pop value
                            }
                        } else if (!strcmp(key, "outputs")) {
                            if (!lua_istable(L, -1))
                                return NULL; // FIXME: lua_close
                            lua_pushnil(L); // space for key
                            while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                const char *s = lua_tostring(L, -1);
                                qDebug() << "output" << s;
                                ret->insertOutput(ret->outputCount(), new NodeOutput(QLatin1String(s)));
                                lua_pop(L, 1); // pop value
                            }
                        } else if (!strcmp(key, "variables")) {
                            if (!lua_istable(L, -1))
                                return NULL; // FIXME: lua_close
                            lua_pushnil(L); // space for key
                            while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                if (!lua_istable(L, -1))
                                    return NULL; // FIXME: lua_close
                                qDebug() << "variable";
                                QMap<QString,QString> kv;
                                lua_pushnil(L); // space for key
                                while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                    const char *key = lua_tostring(L, -2);
                                    const char *value = lua_tostring(L, -1);
                                    qDebug() << key << "==" << value;
                                    kv[QLatin1String(key)] = QLatin1String(value);
                                    lua_pop(L, 1); // pop value
                                }
                                ret->insertVariable(ret->variableCount(),
                                                    new ScriptVariable(kv[QLatin1String("type")],
                                                    kv[QLatin1String("name")],
                                        kv[QLatin1String("value")]));
                                lua_pop(L, 1); // pop value
                            }
                        }
                        lua_pop(L, 1); // pop value
                    }
                }
            }
        }
        lua_close(L);
        return ret;
    }

    return NULL;
}
