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

#include "scriptmanager.h"

#include "node.h"
#include "project.h"
#include "projectreader.h"

#include <QDir>
#include <QFileInfo>

SINGLETON_IMPL(ScriptManager)

ScriptManager::ScriptManager(QObject *parent) :
    QObject(parent)
{
    connect(&mFileSystemWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));

    mChangedFilesTimer.setInterval(500);
    mChangedFilesTimer.setSingleShot(true);
    connect(&mChangedFilesTimer, SIGNAL(timeout()),
            SLOT(fileChangedTimeout()));
}

ScriptInfo *ScriptManager::scriptInfo(const QString &fileName, const QString &relativeTo)
{
    QString path = canonicalPath(fileName, relativeTo);
    if (path.isEmpty())
        return 0;

    if (mScriptInfo.contains(path))
        return mScriptInfo[path];

    ScriptNode *node = loadScript(path);
    if (!node)
        return 0;

    ScriptInfo *info = new ScriptInfo;
    info->mPath = path;
    info->mNode = node;
    mScriptInfo[path] = info;
    mFileSystemWatcher.addPath(path);

    return info;
}

QString ScriptManager::canonicalPath(const QString &fileName, const QString &relativeTo)
{
    QString path = fileName;

    if (QDir::isRelativePath(fileName)) {
        Q_ASSERT(!relativeTo.isEmpty());
        Q_ASSERT(!QDir::isRelativePath(relativeTo));
        path = relativeTo + QLatin1Char('/') + fileName;
    }

    if (!fileName.endsWith(QLatin1String(".pzs")))
        path += QLatin1String(".pzs");

    QFileInfo info(path);
    if (info.exists())
        return info.canonicalFilePath();

    return QString();
}

void ScriptManager::fileChanged(const QString &path)
{
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();
}

void ScriptManager::fileChangedTimeout()
{
    foreach (const QString &path, mChangedFiles) {
        if (mScriptInfo.contains(path)) {
            noise() << "ScriptManager::fileChanged" << path;
            mFileSystemWatcher.removePath(path);
            QFileInfo info(path);
            if (info.exists()) {
                mFileSystemWatcher.addPath(path);
                ScriptInfo *scriptInfo = mScriptInfo[path];
                delete scriptInfo->mNode;
                scriptInfo->mNode = loadScript(path);
                if (!scriptInfo->mNode) {
                    // ???
                }
                emit infoChanged(scriptInfo);
            }
        }
    }

    mChangedFiles.clear();
}

ScriptNode *ScriptManager::loadScript(const QString &path)
{
    ProjectReader reader;
    if (Project *project = reader.read(path)) {
        ScriptNode *node = project->mRootNode;
        project->mRootNode = 0;
        delete project;
        return node;
    }
    return 0;
}
