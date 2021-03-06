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

#include "luautils.h"
#include "node.h"
#include "preferences.h"
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

    connect(prefs(), SIGNAL(gameDirectoriesChanged()), SLOT(gameDirectoriesChanged()));
}

LuaInfo *LuaManager::luaInfo(const QString &fileName, const QString &relativeTo)
{
    QString path = canonicalPath(fileName, relativeTo);
    if (path.isEmpty())
        return 0;

    if (mLuaInfo.contains(path) && mLuaInfo[path]->node())
        return mLuaInfo[path];

    LuaNode *node = loadLua(path);
    if (!node)
        return 0;

    LuaInfo *info = mLuaInfo[path] ? mLuaInfo[path] : new LuaInfo;
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

bool LuaManager::readLuaFiles()
{
    QStringList filters;
    filters << QLatin1String("*.lua");

    foreach (QString path, prefs()->gameDirectories()) {
        QDir dir = QDir(path).filePath(QLatin1String("media/lua/MetaGame"));
        if (!dir.exists()) {
            qDebug() << "ignoring missing game/mod directory" << dir.absolutePath();
            continue;
        }
        foreach (QFileInfo fileInfo, dir.entryInfoList(filters)) {
            if (LuaInfo *info = luaInfo(fileInfo.absoluteFilePath()))
                if (!mCommands.contains(info))
                    mCommands += info;
        }
    }

    return true;
}

void LuaManager::gameDirectoriesChanged()
{
    readLuaFiles();
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
                emit infoChanged(info);
            }
        }
    }

    mChangedFiles.clear();
}

LuaNode *LuaManager::loadLua(const QString &fileName)
{
    if (!QFileInfo(fileName).exists())
        return NULL;

#if 1
    LuaState L;
    if (!L.loadFile(fileName))
        return NULL;

    LuaValue lv = L.getGlobal(QLatin1String("editor"));
    if (lv.type() != LUA_TTABLE)
        return NULL;

    LuaNode node(0, QFileInfo(fileName).baseName());

    for (int i = 0; i < lv.mTableValue.size(); i++) {
        LuaValue *k = lv.mTableValue.mKeys[i];
        LuaValue *v = lv.mTableValue.mValues[i];
        if (k->isString(QLatin1String("inputs"))) {
            if (!v->isTable()) return NULL;
            for (int j = 0; j < v->mTableValue.size(); j++) {
                LuaValue *k2 = v->mTableValue.mKeys[j];
                LuaValue *v2 = v->mTableValue.mValues[j];
                if (!v2->isTable()) return false;
                QMap<QString,QString> ssm = v2->mTableValue.toStringStringMap();
                QString name = ssm[QLatin1String("name")];
                if (node.input(name)) return false;
                QString label = ssm[QLatin1String("label")];
                if (label.isEmpty()) label = name;
                node.insertInput(node.inputCount(), new NodeInput(name, label));
            }
        }
        if (k->isString(QLatin1String("outputs"))) {
            if (!v->isTable()) return NULL;
            for (int j = 0; j < v->mTableValue.size(); j++) {
                LuaValue *k2 = v->mTableValue.mKeys[j];
                LuaValue *v2 = v->mTableValue.mValues[j];
                if (!v2->isTable()) return false;
                QMap<QString,QString> ssm = v2->mTableValue.toStringStringMap();
                QString name = ssm[QLatin1String("name")];
                if (node.output(name)) return false;
                QString label = ssm[QLatin1String("label")];
                if (label.isEmpty()) label = name;
                node.insertOutput(node.outputCount(), new NodeOutput(name, label));
            }
        }
        if (k->isString(QLatin1String("variables"))) {
            if (!v->isTable()) return NULL;
            for (int j = 0; j < v->mTableValue.size(); j++) {
                LuaValue *k2 = v->mTableValue.mKeys[j];
                LuaValue *v2 = v->mTableValue.mValues[j];
                if (!v2->isTable()) return false;
                QMap<QString,QString> ssm = v2->mTableValue.toStringStringMap();
                QString name = ssm[QLatin1String("name")];
                if (node.variable(name)) return false;
                QString type = ssm[QLatin1String("type")];
                QString label = ssm[QLatin1String("label")];
                if (label.isEmpty()) label = name;
                QString value = ssm[QLatin1String("value")];
                node.insertVariable(node.variableCount(), new ScriptVariable(type, name, label, value));
            }
        }
    }

    return new LuaNode(0, node);
#else
    if (lua_State *L = luaL_newstate()) {

        LuaNode *ret = 0;

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
                    ret = new LuaNode(0, QFileInfo(fileName).baseName());
                    lua_pushnil(L); // push space on stack for the key
                    while (lua_next(L, tblidx) != 0) { // pop a key, push key then value
                        const char *key = lua_isstring(L, -2) ? lua_tostring(L, -2) : "";
                        if (!strcmp(key, "inputs")) {
                            if (!lua_istable(L, -1))
                                return NULL; // FIXME: lua_close
                            lua_pushnil(L); // space for key
                            while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                const char *s = lua_tostring(L, -1);
                                ret->insertInput(ret->inputCount(), new NodeInput(QLatin1String(s)));
                                lua_pop(L, 1); // pop value
                            }
                        } else if (!strcmp(key, "outputs")) {
                            if (!lua_istable(L, -1))
                                return NULL; // FIXME: lua_close
                            lua_pushnil(L); // space for key
                            while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                const char *s = lua_tostring(L, -1);
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
                                QMap<QString,QString> kv;
                                lua_pushnil(L); // space for key
                                while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                    const char *key = lua_tostring(L, -2);
                                    const char *value = lua_tostring(L, -1);
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
#endif
    return NULL;
}
