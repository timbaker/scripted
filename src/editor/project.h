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

#ifndef PROJECT_H
#define PROJECT_H

#include "editor_global.h"

#include <QList>

class Project
{
public:
    Project();
    ~Project();

    ScriptNode *rootNode() const
    {
        return mRootNode;
    }

    ScriptVariable *resolveVariable(const QString &name);

    int mNextID; // must come before mRootNode
    ScriptNode* mRootNode;
};

#endif // PROJECT_H
