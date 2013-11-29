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

#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include "singleton.h"

#include <QObject>
#include <QToolBar>

class BaseGraphicsScene;
class Document;

class QAction;
class QActionGroup;
class QToolBar;

class AbstractTool;

class ToolManager : public QObject, public Singleton<ToolManager>
{
    Q_OBJECT

public:
    ToolManager();
    ~ToolManager();

    void addTool(AbstractTool *tool);

    const QList<AbstractTool*> &tools() const
    { return mTools; }

    void activateTool(AbstractTool *tool);


    AbstractTool *currentTool() const
    { return mCurrentTool; }

    void clearDocument();
    void setScene(BaseGraphicsScene *scene);

    void beginClearScene(BaseGraphicsScene *scene);
    void endClearScene(BaseGraphicsScene *scene);

    void languageChanged();
signals:
    void currentSceneChanged();
    void currentToolChanged(AbstractTool *tool);
    void statusTextChanged(AbstractTool *tool);

private slots:
    void currentToolStatusTextChanged();
    void toolEnabledChanged(bool enabled);

private:
    QList<AbstractTool*> mTools;
    AbstractTool *mCurrentTool;
    BaseGraphicsScene *mCurrentScene;
};

#endif // TOOLMANAGER_H
