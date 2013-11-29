/*
 * toolmanager.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
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

#include "toolmanager.h"

#include "abstracttool.h"
#include "basegraphicsscene.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QEvent>

SINGLETON_IMPL(ToolManager)

ToolManager::ToolManager() :
    QObject(),
    mCurrentTool(0),
    mCurrentScene(0)
{
}

ToolManager::~ToolManager()
{
}

void ToolManager::addTool(AbstractTool *tool)
{
    mTools += tool;
    connect(tool, SIGNAL(enabledChanged(bool)), SLOT(toolEnabledChanged(bool)));
}

void ToolManager::activateTool(AbstractTool *tool)
{
    if (mCurrentTool) {
        mCurrentTool->deactivate();
        mCurrentTool->setScene(0);
        mCurrentTool->action()->setChecked(false);
        disconnect(mCurrentTool, SIGNAL(statusTextChanged()),
                   this, SLOT(currentToolStatusTextChanged()));
    }

    mCurrentTool = tool;

    if (mCurrentTool) {
        connect(mCurrentTool, SIGNAL(statusTextChanged()),
                SLOT(currentToolStatusTextChanged()));
        Q_ASSERT(mCurrentScene != 0);
        mCurrentTool->setScene(mCurrentScene);
        mCurrentTool->activate();
        mCurrentTool->action()->setChecked(true);
    }

    qDebug() << "ToolManager::mCurrentTool" << mCurrentTool << "mCurrentScene" << mCurrentScene;

    emit currentToolChanged(mCurrentTool);
}

void ToolManager::toolEnabledChanged(bool enabled)
{
    AbstractTool *tool = (AbstractTool*) sender();

    if (enabled && !mCurrentTool) {
        activateTool(tool);
    } else if (!enabled && tool == mCurrentTool) {
        foreach (AbstractTool *tool2, mTools) {
            if (tool2 != tool && tool2->action()->isEnabled()) {
                activateTool(tool2);
                return;
            }
        }
        activateTool(0);
    }
}

void ToolManager::clearDocument()
{
    // Avoid a race condition when a document is closed.
    // When updateActions() calls setEnabled(false) on each tool one-by-one,
    // another tool is activated (see toolEnabledChanged()).
    // No tool should become active when a document is closing.
    foreach (AbstractTool *tool, mTools)
        tool->action()->setEnabled(false);
    activateTool(0);
    mCurrentScene = 0;
}

void ToolManager::setScene(BaseGraphicsScene *scene)
{
    if (mCurrentScene)
        clearDocument();

    mCurrentScene = scene;

    if (mCurrentScene) {

    }

    emit currentSceneChanged();
}

void ToolManager::beginClearScene(BaseGraphicsScene *scene)
{
    if ((scene == mCurrentScene) && mCurrentTool)
        mCurrentTool->beginClearScene();
}

void ToolManager::endClearScene(BaseGraphicsScene *scene)
{
    if ((scene == mCurrentScene) && mCurrentTool)
        mCurrentTool->endClearScene();
}

void ToolManager::languageChanged()
{
    // Allow the tools to adapt to the new language
    foreach (AbstractTool *tool, mTools) {
        tool->languageChanged();

        // Update the text, shortcut and tooltip of the action
        QAction *action = tool->action();
        action->setText(tool->name());
        action->setShortcut(tool->shortcut());
        action->setToolTip(QString(QLatin1String("%1 (%2)")).arg(
                tool->name(), tool->shortcut().toString()));
    }
}


void ToolManager::currentToolStatusTextChanged()
{
}
