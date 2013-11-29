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

#ifndef PROJECTDOCUMENT_H
#define PROJECTDOCUMENT_H

#include "document.h"
#include "editor_global.h"

class ProjectDocument : public Document
{
    Q_OBJECT
public:
    ProjectDocument(Project *prj, const QString &fileName);

    void setFileName(const QString &fileName);
    const QString &fileName() const;
    bool save(const QString &filePath, QString &error);

    Project *project() const
    { return mProject; }

    ProjectChanger *changer() const
    { return mChanger; }

private:
    QString mFileName;
    Project *mProject;
    ProjectChanger *mChanger;
};

#endif // PROJECTDOCUMENT_H
