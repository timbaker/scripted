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

#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include "basegraphicsview.h"

#include "editor_global.h"

class ScriptView : public BaseGraphicsView
{
    Q_OBJECT
public:
    explicit ScriptView(ProjectDocument *doc, QWidget *parent = 0);

    ScriptScene *scene() const { return mScene; }
    void setScene(ScriptScene *scene);

signals:

public slots:

private:
    ScriptScene *mScene;
    ProjectDocument *mDocument;
};

#endif // SCRIPTVIEW_H
