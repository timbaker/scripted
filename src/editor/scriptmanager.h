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

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include "editor_global.h"
#include "filesystemwatcher.h"
#include "singleton.h"

#include <QMap>
#include <QTimer>

class ScriptInfo
{
public:
    const QString &path() const { return mPath; }
    ScriptNode *node() const { return mNode; }

private:
    QString mPath;
    ScriptNode *mNode;

    friend class ScriptManager;
};

class ScriptManager : public QObject, public Singleton<ScriptManager>
{
    Q_OBJECT
public:
    explicit ScriptManager(QObject *parent = 0);

    ScriptInfo *scriptInfo(const QString &fileName, const QString &relativeTo = QString());
    QString canonicalPath(const QString &fileName, const QString &relativeTo = QString());

signals:
    void infoChanged(ScriptInfo *info);

public slots:
    void fileChanged(const QString &path);
    void fileChangedTimeout();

private:
    ScriptNode *loadScript(const QString &path);

private:
    QMap<QString,ScriptInfo*> mScriptInfo;

    FileSystemWatcher mFileSystemWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

inline ScriptManager *scriptmgr() { return ScriptManager::instance(); }

#endif // SCRIPTMANAGER_H
