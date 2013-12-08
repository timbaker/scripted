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

#include "luautils.h"
#include "node.h"
#include "preferences.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>

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

MetaEventInfo *MetaEventManager::info(const QString &source, const QString &eventName)
{
    QString path = source;
    QFileInfo fileInfo(path);
    if (fileInfo.exists())
        path = fileInfo.canonicalFilePath();
    else
        return 0;

    if (mEventsByFile.contains(path) && mEventsByFile[path].contains(eventName)) {
        MetaEventInfo *info = mEventsByFile[path][eventName];
        if (info->node())
            return info;
    }

    if (!readEventFile(path))
        return 0;

    if (mEventsByFile.contains(path) && mEventsByFile[path].contains(eventName)) {
        MetaEventInfo *info = mEventsByFile[path][eventName];
        if (info->node())
            return info;
    }

    return 0;
}

QList<MetaEventInfo *> MetaEventManager::events(const QString &source)
{
    QString path = QDir::cleanPath(source);
    QFileInfo fileInfo(path);
    if (fileInfo.exists())
        path = fileInfo.canonicalFilePath();

    if (mEventsByFile.contains(path))
        return mEventsByFile[path].values();
    return QList<MetaEventInfo*>();
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
    QMap<QString,MetaEventInfo*> oldEvents = mEventsByFile[fileName];
    QMap<QString,MetaEventInfo*> &newEvents = mEventsByFile[fileName];
//    newEvents.clear();

    bool ok = false;

    if (QFileInfo(fileName).exists()) {

        // Watch the file even if it fails to load
        mFileSystemWatcher.addPath(fileName);

        MetaEventFile file;
        if (file.read(fileName)) {
            foreach (MetaEventNode *node, file.takeNodes()) {
                MetaEventInfo *info;
                if (oldEvents.contains(node->eventName())) {
                    info = oldEvents[node->eventName()];
                    delete info->node();
                    oldEvents.remove(node->eventName());
                } else {
                    info = new MetaEventInfo;
                }
                info->mNode = node;
                info->mPath = fileName;
                info->mEventName = node->eventName();
                node->setInfo(info);
                newEvents[info->eventName()] = info;
            }

            foreach (MetaEventInfo *info, newEvents)
                emit infoChanged(info);

            ok = true;
        }
    }

    // Handle events that were in this file but aren't anymore.
    // This also handles a file being deleted.
    foreach (MetaEventInfo *info, oldEvents) {
        delete info->node();
        info->mNode = 0;
        emit infoChanged(info);
    }

    return ok;
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
            // This also handles the file not existing anymore
            readEventFile(QFileInfo(path).canonicalFilePath());
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

    LuaState L;
    if (!L.loadFile(fileName))
        return false;

    LuaValue lv = L.getGlobal(QLatin1String("events"));
    if (lv.type() != LUA_TTABLE)
        return false;

    LuaNode node(0, QFileInfo(fileName).baseName());

    QStringList eventNames;

    for (int i = 0; i < lv.mTableValue.size(); i++) {
//        LuaValue *k = lv.mTableValue.mKeys[i];
        LuaValue *v = lv.mTableValue.mValues[i];
        if (!v->isTable()) return false;
        MetaEventNode node(0, QString(), QString());
        for (int j = 0; j < v->mTableValue.size(); j++) {
            LuaValue *k2 = v->mTableValue.mKeys[j];
            LuaValue *v2 = v->mTableValue.mValues[j];
            if (k2->isString(QLatin1String("name"))) {
                if (v2->mType != LUA_TSTRING) return false;
                node.setEventName(v2->toString());
                node.setLabel(v2->toString());
            }
            if (k2->isString(QLatin1String("variables"))) {
                if (!v2->isTable()) return false;
                for (int m = 0; m < v2->mTableValue.size(); m++) {
//                    LuaValue *k3 = v2->mTableValue.mKeys[m];
                    LuaValue *v3 = v2->mTableValue.mValues[m];
                    if (!v3->isTable()) return false;
                    QMap<QString,QString> ssm = v3->mTableValue.toStringStringMap();
                    QString name = ssm[QLatin1String("name")];
                    QString type = ssm[QLatin1String("type")];
                    QString label = ssm[QLatin1String("label")];
                    if (label.isEmpty()) label = name;
                    QString value = ssm[QLatin1String("value")];
                    node.insertVariable(node.variableCount(), new ScriptVariable(type, name, label, value));
                }
            }
        }
        if (node.eventName().isEmpty()) return false;
        if (eventNames.contains(node.eventName())) return false;
        eventNames += node.eventName();
        node.setSource(fileName);
        mNodes += new MetaEventNode(0, node);
    }

    return new LuaNode(0, node);
}

QList<MetaEventNode *> MetaEventFile::takeNodes()
{
    QList<MetaEventNode *> nodes = mNodes;
    mNodes.clear();
    return nodes;
}
