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

#include "metaeventmanager.h"

#include "node.h"
#include "preferences.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>

extern "C" {

#include "lualib.h"
#include "lauxlib.h"

} // extern "C"

SINGLETON_IMPL(MetaEventManager)

MetaEventManager::MetaEventManager(QObject *parent) :
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

MetaEventInfo *MetaEventManager::info(const QString &eventName)
{
    if (mEventInfo.contains(eventName))
        return mEventInfo[eventName];

    return 0;
}

bool MetaEventManager::readEventFiles()
{
    QStringList filters;
    filters << QLatin1String("MetaEvents.lua");
    foreach (QString path, prefs()->gameDirectories()) {
        QDir dir = QDir(path).filePath(QLatin1String("media/lua/MetaGame"));
        if (!dir.exists()) {
            qDebug() << "ignoring missing game/mod directory" << dir.absolutePath();
            continue;
        }
        foreach (QFileInfo fileInfo, dir.entryInfoList(filters)) {
            QString fileName = fileInfo.canonicalFilePath();
            readEventFile(fileName);
        }
    }
    return true;
}

bool MetaEventManager::readEventFile(const QString &fileName)
{
    MetaEventFile file;
    if (file.read(fileName)) {
        QList<MetaEventInfo*> oldEvents = mEventsByFile[fileName];
        QList<MetaEventInfo*> &newEvents = mEventsByFile[fileName];
        newEvents.clear();
        foreach (MetaEventNode *node, file.takeNodes()) {
            MetaEventInfo *info = mEventInfo[node->name()];
            if (info) {
                delete info->node();
                node->setInfo(info);
                info->mNode = node;
                info->mPath = fileName;
                emit infoChanged(info);
            } else {
                info = new MetaEventInfo;
                info->mNode = node;
                info->mPath = fileName;
                node->setInfo(info);
                mEventInfo[node->name()] = info;
                emit infoChanged(info);
            }
            newEvents += info;
        }
        // FIXME: an event with the same name could have come from another file
        foreach (MetaEventInfo *info, oldEvents) {
            if (!newEvents.contains(info)) {
                delete info->node();
                info->mNode = 0;
                emit infoChanged(info);
            }
        }

        mFileSystemWatcher.addPath(fileName);
        return true;
    }
    return false;
}

void MetaEventManager::gameDirectoriesChanged()
{
    readEventFiles();
}

void MetaEventManager::fileChanged(const QString &path)
{
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();
}

void MetaEventManager::fileChangedTimeout()
{
    foreach (const QString &path, mChangedFiles) {
        if (true) {
            noise() << "MetaEventManager::fileChanged" << path;
            mFileSystemWatcher.removePath(path);
            QFileInfo info(path);
            if (info.exists()) {
                readEventFile(QFileInfo(path).canonicalFilePath());
            }
        }
    }

    mChangedFiles.clear();
}

/////

MetaEventFile::MetaEventFile()
{

}

MetaEventFile::~MetaEventFile()
{
    qDeleteAll(mNodes);
}

bool MetaEventFile::read(const QString &fileName)
{
    qDeleteAll(mNodes);
    mNodes.clear();

    if (!QFileInfo(fileName).exists())
        return false;

    if (lua_State *L = luaL_newstate()) {
        luaL_openlibs(L);
        int status = luaL_loadfile(L, fileName.toLatin1().data());
        if (status == LUA_OK) {
            status = lua_pcall(L, 0, 0, -1); // call the closure?
            if (status == LUA_OK) {
                lua_getglobal(L, "events");
                int tblidx = lua_gettop(L);
                if (lua_istable(L, tblidx)) { // events = { ... }
                    lua_pushnil(L); // push space on stack for the key
                    while (lua_next(L, tblidx) != 0) { // pop a key, push key then value
                        MetaEventNode *node = new MetaEventNode(0, QString());
                        if (!lua_istable(L, -1))
                            return false; // FIXME: lua_close
                        lua_pushnil(L); // space for key
                        while (lua_next(L, -2) != 0) { // pop a key, push key then value
                            const char *key = lua_isstring(L, -2) ? lua_tostring(L, -2) : "";
                            if (!strcmp(key, "name")) {
                                node->setName(lua_tostring(L, -1));
                            } else if (!strcmp(key, "variables")) {
                                if (!lua_istable(L, -1))
                                    return false; // FIXME: lua_close
                                lua_pushnil(L); // space for key
                                while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                    if (!lua_istable(L, -1))
                                        return false; // FIXME: lua_close
                                    QMap<QString,QString> kv;
                                    lua_pushnil(L); // space for key
                                    while (lua_next(L, -2) != 0) { // pop a key, push key then value
                                        const char *key = lua_tostring(L, -2);
                                        const char *value = lua_tostring(L, -1);
                                        kv[QLatin1String(key)] = QLatin1String(value);
                                        lua_pop(L, 1); // pop value
                                    }
                                    node->insertVariable(node->variableCount(),
                                                         new ScriptVariable(kv[QLatin1String("type")],
                                                         kv[QLatin1String("name")],
                                            kv[QLatin1String("value")]));
                                    lua_pop(L, 1); // pop value
                                }
                            }
                            lua_pop(L, 1); // pop value
                        }
                        lua_pop(L, 1); // pop value
                        if (node->name().isEmpty())
                            return false;
                        mNodes += node;
                    }
                }
            }
        }
        lua_close(L);
        return true;
    }

    return false;
}

QList<MetaEventNode *> MetaEventFile::takeNodes()
{
    QList<MetaEventNode *> nodes = mNodes;
    mNodes.clear();
    return nodes;
}
