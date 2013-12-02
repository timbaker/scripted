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

#ifndef METAEVENTMANAGER_H
#define METAEVENTMANAGER_H

#include "editor_global.h"
#include "filesystemwatcher.h"
#include "singleton.h"
#include <QTimer>

class MetaEventInfo
{
public:
    MetaEventNode *node() const { return mNode; }

private:
    QString mPath; // the MetaEvents.lua file we came from
    MetaEventNode *mNode;

    friend class MetaEventManager;
};

class MetaEventManager : public QObject, public Singleton<MetaEventManager>
{
    Q_OBJECT
public:
    explicit MetaEventManager(QObject *parent = 0);

    MetaEventInfo *info(const QString &eventName);
    QList<MetaEventInfo*> events() { return mEventInfo.values(); }
    bool readLuaFiles();

signals:
    void eventChanged(MetaEventInfo *info);

public slots:
    void fileChanged(const QString &path);
    void fileChangedTimeout();

private:
    bool loadEvents(const QString &fileName);

private:
    QMap<QString,MetaEventInfo*> mEventInfo;

    FileSystemWatcher mFileSystemWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

inline MetaEventManager *eventmgr() { return MetaEventManager::instance(); }

#endif // METAEVENTMANAGER_H
