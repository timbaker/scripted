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

#include "abstracttool.h"

#include "toolmanager.h"

#include <QAction>

AbstractTool::AbstractTool(const QString &name, const QIcon &icon,
                           const QKeySequence &shortcut,
                           QObject *parent)
    : QObject(parent)
    , mName(name)
    , mIcon(icon)
    , mShortcut(shortcut)
    , mAction(0)
{
}

void AbstractTool::setAction(QAction *action)
{
    mAction = action;
    mAction->setIcon(mIcon);
    connect(mAction, SIGNAL(triggered()), SLOT(makeCurrent()));
}

void AbstractTool::setStatusText(const QString &text)
{
    mStatusText = text;
    emit statusTextChanged();
}

bool AbstractTool::isEnabled() const
{
    return mAction->isEnabled();
}

void AbstractTool::setEnabled(bool enabled)
{
    if (enabled == mAction->isEnabled())
        return;

    mAction->setEnabled(enabled);
    emit enabledChanged(enabled);
}

bool AbstractTool::isCurrent() const
{
    return ToolManager::instance()->currentTool() == this;
}

void AbstractTool::makeCurrent()
{
    ToolManager::instance()->activateTool(this);
}
