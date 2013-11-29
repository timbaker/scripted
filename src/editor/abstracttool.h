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

#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

#include "editor_global.h"

#include <QObject>
#include <QIcon>
#include <QKeySequence>
#include <QMetaType>

class BaseImageTool;
class BasePathTool;
class BasePaintTool;

class QAction;
class QGraphicsSceneMouseEvent;
class QKeyEvent;

class AbstractTool : public QObject
{
    Q_OBJECT
public:
    AbstractTool(const QString &name,
                 const QIcon &icon,
                 const QKeySequence &shortcut,
                 QObject *parent = 0);

    virtual ~AbstractTool() {}

    void setAction(QAction *action);

    QAction *action() const
    { return mAction; }

    virtual void activate() {}
    virtual void deactivate() {}

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QIcon icon() const { return mIcon; }
    void setIcon(const QIcon &icon) { mIcon = icon; }

    QKeySequence shortcut() const { return mShortcut; }
    void setShortcut(const QKeySequence &shortcut) { mShortcut = shortcut; }

    QString statusText() const { return mStatusText; }
    void setStatusText(const QString &text);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isCurrent() const;

    virtual BaseImageTool *asImageTool() { return 0; }
    virtual BasePaintTool *asPaintTool() { return 0; }
    virtual BasePathTool *asPathTool() { return 0; }

    virtual void languageChanged() = 0;

    virtual void setScene(BaseGraphicsScene *scene) = 0;
    virtual BaseGraphicsScene *scene() const = 0;

    virtual void beginClearScene() { Q_ASSERT(scene()); deactivate(); }
    virtual void endClearScene() { Q_ASSERT(scene()); activate(); }

    virtual void keyPressEvent(QKeyEvent *event) { Q_UNUSED(event) }
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event) }
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event) }
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event) }

signals:
    void statusTextChanged();
    void enabledChanged(bool enabled);

public slots:
    virtual void updateEnabledState() = 0;
    void makeCurrent();

private:
    QString mName;
    QIcon mIcon;
    QKeySequence mShortcut;
    QString mStatusText;
    QAction *mAction;
};

// Needed for QVariant handling
Q_DECLARE_METATYPE(AbstractTool*)

#endif // ABSTRACTTOOL_H
