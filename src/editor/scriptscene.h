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

struct InputOrOutputItem
{
    InputOrOutputItem() : input(0), output(0) {}
    bool isValid() { return input != 0 || output != 0; }
    void clear() { input = 0; output = 0; }
    BaseNode *node();
    QString name();
    void setHighlight(bool hl);
    bool operator ==(const InputOrOutputItem &other)
    { return input == other.input && output == other.output; }
    bool operator !=(const InputOrOutputItem &other)
    { return input != other.input || output != other.output; }
    QGraphicsItem *item();
    QPointF connectPosLeft();
    QPointF connectPosRight();

    NodeInputItem *input;
    NodeOutputItem *output;
};

class ConnectionItem : public QGraphicsItem
{
public:
    ConnectionItem(NodeConnection *cxn, InputOrOutputItem from,
                   InputOrOutputItem to, QGraphicsItem *parent = 0);

    void updateBounds();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QPolygonF allPoints() const;
    QPolygonF controlPoints() const;

    static const int NODE_RADIUS = 8;

    QRectF mBounds;
    QPainterPath mShape;
    NodeConnection *mConnection;
    InputOrOutputItem mConnectFrom;
    InputOrOutputItem mConnectTo;
    bool mShowNodes;
    int mControlPointIndex;
    int mHighlightIndex;
    QPointF mControlPointDragPos;
    bool mMouseMoved;
    bool mAddingNewPoint;
    QPointF mStartPoint;
    QPointF mEndPoint;
};

class ConnectionsItem : public QGraphicsItem
{
public:
    ConnectionsItem(ProjectScene *scene, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    void updateConnections();
    ConnectionItem *itemFor(NodeConnection *cxn);

    void moved(NodeInputItem *item);
    void moved(NodeOutputItem *item);

    void newConnectionStart(const QPointF &scenePos);
    void newConnectionHotspot(const QPointF &scenePos);
    void newConnectionClick(const QPointF &scenePos);
    void newConnectionEnd();
    void newConnectionCancel();

    void afterSetControlPoints(NodeConnection *cxn);

    ProjectScene *mScene;
    QList<ConnectionItem*> mConnectionItems;

    QPolygonF mNewConnectionPoints;
    QGraphicsPathItem *mNewConnectionItem;
    static bool mMakingConnection;
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
    const QList<NodeItem*> &nodeItems() const
    {
        return mNodeItems;
    }

    QRectF boundsOfAllNodes();

    NodeInputItem *rootInputItem(const QString &name);
    NodeOutputItem *rootOutputItem(const QString &name);

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
    void afterRenameNode(BaseNode *node, const QString &oldName);

    void inputsChanged(BaseNode *node);
    void outputsChanged(BaseNode *node);
    void inputsChanged();
    void outputsChanged();

    void afterAddConnection(int index, NodeConnection *cxn);
    void afterRemoveConnection(int index, NodeConnection *cxn);
    void afterSetControlPoints(NodeConnection *cxn, const QPolygonF &oldPoints);

    void afterChangeVariable(ScriptVariable *var, const ScriptVariable *oldValue);
    void afterAddVariable(BaseNode *node, int index, ScriptVariable *var);
    void afterRemoveVariable(BaseNode *node, int index, ScriptVariable *var);

    void infoChanged(MetaEventInfo *info);
    void infoChanged(ScriptInfo *info);
    void infoChanged(LuaInfo *info);

private:
    ProjectDocument *mDocument;
    QList<NodeItem*> mNodeItems;
    ConnectionsItem *mConnectionsItem;
    InputOrOutputItem mConnectFrom;
    InputOrOutputItem mConnectTo;
    GridItem *mGridItem;
    ScriptAreaItem *mAreaItem;
    bool mDragHasPZS;

    friend class ScriptAreaItem;
};

#endif // SCRIPTSCENE_H
