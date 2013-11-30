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

#ifndef SCRIPTVARIABLE_H
#define SCRIPTVARIABLE_H

#include "editor_global.h"

#include <QList>
#include <QString>

class ScriptVariable
{
public:
    explicit ScriptVariable(const QString &type, const QString &name,
                            const QString &value, const QString &variableRef);
    ScriptVariable(const ScriptVariable *other);

    void setNode(BaseNode *node) { mNode = node; }
    BaseNode *node() const { return mNode; }

    void setType(const QString &type) { mType = type; }
    const QString &type() const { return mType; }

    void setName(const QString &name) { mName = name; }
    const QString &name() const { return mName; }

    void setValue(const QString &value) { mValue = value; }
    const QString &value() const { return mValue; }

    void setVariableRef(const QString &varName) { mVariableRef = varName; }
    const QString &variableRef() const { return mVariableRef; }

    bool isKnown() const;

private:
    BaseNode *mNode;
    QString mName; // --> User-assigned name in a script
    QString mType; // --> Number, String, Actor, etc
    QString mValue; // User-defined value, ignored if mVariableRef is valid
    QString mVariableRef; // Name of a variable containing the value to use
};

#endif // SCRIPTVARIABLE_H
