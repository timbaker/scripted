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

#include "document.h"

#include "luadocument.h"
#include "projectdocument.h"

#include <QFileInfo>
#include <QUndoStack>

Document::Document(DocumentType type)
    : QObject()
    , mUndoStack(0)
    , mType(type)
//    , mView(0)
{
}

LuaDocument *Document::asLuaDocument()
{
    return isLuaDocument() ? static_cast<LuaDocument*>(this) : NULL;
}

ProjectDocument *Document::asProjectDocument()
{
    return isProjectDocument() ? static_cast<ProjectDocument*>(this) : NULL;
}

bool Document::isModified() const
{
    return mUndoStack->isClean() == false;
}

QString Document::displayName() const
{
    QString displayName = QFileInfo(fileName()).fileName();
    if (displayName.isEmpty())
        displayName = tr("untitled.iep");
    return displayName;
}
