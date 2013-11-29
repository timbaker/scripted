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

#include "projectdocument.h"

#include "projectchanger.h"
#include "projectwriter.h"

#include <QUndoStack>

ProjectDocument::ProjectDocument(Project *prj, const QString &fileName) :
    Document(ProjectDocType),
    mProject(prj),
    mChanger(new ProjectChanger(prj)),
    mFileName(fileName)
{
    mUndoStack = new QUndoStack(this);
}

void ProjectDocument::setFileName(const QString &fileName)
{
    if (fileName == mFileName)
        return;
    mFileName = fileName;
    emit fileNameChanged();
}

const QString &ProjectDocument::fileName() const
{
    return mFileName;
}

bool ProjectDocument::save(const QString &filePath, QString &error)
{
    ProjectWriter writer;
    if (!writer.write(project(), filePath)) {
        error = writer.errorString();
        return false;
    }

    error.clear();
    undoStack()->setClean();
    setFileName(filePath);
    return true;
}
