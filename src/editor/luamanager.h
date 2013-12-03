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

#ifndef LUAMANAGER_H
#define LUAMANAGER_H

#include "editor_global.h"
#include "filesystemwatcher.h"
#include "singleton.h"

#include <Map>
#include <QTimer>

class LuaInfo
{
public:
    const QString &path() const { return mPath; }
    LuaNode *node() const { return mNode; }

private:
    QString mPath;
    LuaNode *mNode;

    friend class LuaManager;
};

class LuaManager : public QObject, public Singleton<LuaManager>
{
    Q_OBJECT
public:
    explicit LuaManager(QObject *parent = 0);

    LuaInfo *luaInfo(const QString &fileName, const QString &relativeTo = QString());
    QString canonicalPath(const QString &fileName, const QString &relativeTo = QString());

    const QList<LuaInfo*> &commands() const
    { return mCommands; }

    bool readLuaFiles();

signals:
    void luaChanged(LuaInfo *info);

public slots:
    void gameDirectoriesChanged();
    void fileChanged(const QString &path);
    void fileChangedTimeout();

private:
    LuaNode *loadLua(const QString &fileName);

private:
    QMap<QString,LuaInfo*> mLuaInfo;
    QList<LuaInfo*> mCommands;

    FileSystemWatcher mFileSystemWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

inline LuaManager *luamgr() { return LuaManager::instance(); }

#endif // LUAMANAGER_H
