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

#ifndef LUADOCUMENT_H
#define LUADOCUMENT_H

#include "document.h"

class LuaEditor;

class LuaDocument : public Document
{
public:
    LuaDocument(const QString &fileName);

    bool isModified() const;

    void setFileName(const QString &fileName);
    const QString &fileName() const;
    QString extension() const { return QLatin1String(".lua"); }
    QString filter() const { return tr("Lua files (*.lua)"); }
    bool save(const QString &filePath, QString &error);

    void setEditor(LuaEditor *editor);

private:
    QString mFileName;
    LuaEditor *mEditor;
};

#endif // LUADOCUMENT_H
