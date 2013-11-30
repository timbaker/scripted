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

#ifndef SCRIPTSCENE_H
#define SCRIPTSCENE_H

#include "editor_global.h"
#include "basegraphicsscene.h"

#include <QGraphicsItem>

class ConnectionItem : public QGraphicsItem
{
public:
    ConnectionItem(NodeOutputItem *outItem, NodeInputItem *inItem, QGraphicsItem *parent = 0);

    void updateBounds();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QRectF mBounds;
    NodeOutputItem *mOutputItem;
    NodeInputItem *mInputItem;
};

class ConnectionsItem : public QGraphicsItem
{
public:
    ConnectionsItem(ProjectScene *scene, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    void updateConnections();

    void moved(NodeInputItem *item);
    void moved(NodeOutputItem *item);

    ProjectScene *mScene;
    QList<ConnectionItem*> mConnectionItems;
};

class GridItem : public QGraphicsItem
{
public:
    GridItem(ProjectScene *scene, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return mBounds; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void updateBounds();

    QRectF mBounds;
};

// Contains all the nodes in the scene.
// Has input/output items as children.
class ScriptAreaItem : public QGraphicsItem
{
public:
    ScriptAreaItem(ProjectScene *scene, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void updateBounds();

    ProjectScene *mScene;
    QRectF mBounds;
    NodeInputGroupItem *mInputsItem;
    NodeOutputGroupItem *mOutputsItem;
};

class ScriptScene : public BaseGraphicsScene
{
    Q_OBJECT
public:
    explicit ScriptScene(ProjectDocument *doc, QObject *parent = 0);

    void setTool(AbstractTool *tool);

    ProjectDocument *document() const { return mDocument; }

    NodeItem *createItemForNode(BaseNode *node);
    NodeItem *itemForNode(BaseNode *node);
    const QList<NodeItem*> &objectItems() const
    {
        return mNodeItems;
    }

    QRectF boundsOfAllNodes();

    void moved(NodeInputItem *item);
    void moved(NodeOutputItem *item);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

signals:

public slots:
    void sceneRectChanged();

    void afterAddNode(int index, BaseNode *node);
    void afterRemoveNode(int index, BaseNode *node);
    void afterMoveNode(BaseNode *node, const QPointF &oldPos);

    void afterAddConnection(int index, NodeConnection *cxn);
    void afterRemoveConnection(int index, NodeConnection *cxn);

    void afterSetVariableValue(ScriptVariable *var, const QString &oldValue);

    void scriptChanged(ScriptInfo *info);

private:
    ProjectDocument *mDocument;
    QList<NodeItem*> mNodeItems;
    ConnectionsItem *mConnectionsItem;
    NodeOutputItem *mConnectFrom;
    QGraphicsLineItem *mConnectFeedbackItem;
    NodeInputItem *mConnectTo;
    GridItem *mGridItem;
    ScriptAreaItem *mAreaItem;
    bool mDragHasPZS;
};

#endif // SCRIPTSCENE_H
