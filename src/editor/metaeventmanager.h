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
    QString path() const { return mPath; }
    QString eventName() const { return mEventName; }
    MetaEventNode *node() const { return mNode; }

private:
    QString mPath; // the MetaEvents.lua file we came from
    QString mEventName;
    MetaEventNode *mNode;

    friend class MetaEventManager;
};

class MetaEventFile
{
public:
    MetaEventFile();
    ~MetaEventFile();

    bool read(const QString &fileName);
    QList<MetaEventNode*> takeNodes();

    QString errorString() { return mError; }

private:
    QList<MetaEventNode*> mNodes;
    QString mError;
};

class MetaEventManager : public QObject, public Singleton<MetaEventManager>
{
    Q_OBJECT
public:
    explicit MetaEventManager(QObject *parent = 0);

    MetaEventInfo *info(const QString &source, const QString &eventName);
    QList<MetaEventInfo*> events(const QString &source);
    bool readEventFiles();

signals:
    void infoChanged(MetaEventInfo *info);

private:
    bool readEventFile(const QString &fileName);

public slots:
    void gameDirectoriesChanged();
    void fileChanged(const QString &path);
    void fileChangedTimeout();

private:
    QMap<QString,QMap<QString,MetaEventInfo*> > mEventsByFile;

    FileSystemWatcher mFileSystemWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

inline MetaEventManager *eventmgr() { return MetaEventManager::instance(); }

#endif // METAEVENTMANAGER_H
