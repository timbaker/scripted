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
#include "singleton.h"

#include <QObject>

class LuaManager : public QObject, public Singleton<LuaManager>
{
    Q_OBJECT
public:
    explicit LuaManager(QObject *parent = 0);

    const QList<LuaNodeDef*> &commands() const
    { return mCommands; }
    LuaNodeDef *command(const QString &path);

    bool readLuaFiles();

signals:

public slots:

private:
    QList<LuaNodeDef*> mCommands;

};

#endif // LUAMANAGER_H
