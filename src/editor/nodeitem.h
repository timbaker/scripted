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

#ifndef DRAFTOBJECTITEM_H
#define DRAFTOBJECTITEM_H

#include "editor_global.h"
#include <QGraphicsItem>

class NodeInputItem : public QGraphicsItem
{
public:
    NodeInputItem(ScriptScene *scene, NodeInput *pin, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static QSize size() { return QSize(22, 28); }

    ScriptScene *mScene;
    NodeInput *mInput;
    bool mConnectHighlight;
};

class NodeOutputItem : public QGraphicsItem
{
public:
    NodeOutputItem(ScriptScene *scene, NodeOutput *pin, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static QSize size() { return QSize(22, 28); }

    ScriptScene *mScene;
    NodeOutput *mOutput;
};

class NodeInputGroupItem : public QGraphicsItem
{
public:
    NodeInputGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    void updateLayout();

    void added(int index);
    void removed(int index);
    NodeInputItem *itemFor(NodeInput *input);

    ScriptScene *mScene;
    BaseNode *mNode;
    QList<NodeInputItem*> mItems;
};

class NodeOutputGroupItem : public QGraphicsItem
{
public:
    NodeOutputGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    void updateLayout();

    void added(int index);
    void removed(int index);
    NodeOutputItem *itemFor(NodeOutput *output);

    ScriptScene *mScene;
    BaseNode *mNode;
    QList<NodeOutputItem*> mItems;
};

class BaseVariableItem : public QGraphicsItem
{
public:
    BaseVariableItem(NodeItem *parent, ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

    QStringList getDropData(QGraphicsSceneDragDropEvent *event);

    NodeItem *mParent;
    ScriptVariable *mVariable;
    bool mDropHighlight;
};

class ActorVariableItem : public BaseVariableItem
{
public:
    ActorVariableItem(NodeItem *parent, ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class IntegerVariableItem : public BaseVariableItem
{
public:
    IntegerVariableItem(NodeItem *parent, ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class StringVariableItem : public BaseVariableItem
{
public:
    StringVariableItem(NodeItem *parent, ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};


class NodeItem : public QGraphicsItem
{
public:
    NodeItem(ProjectScene *scene, BaseNode *obj, QGraphicsItem *parent = 0);

    BaseNode *node() const { return mNode; }
    NodeInputItem *inputItem(NodeInput *input);
    NodeOutputItem *outputItem(NodeOutput *output);
    BaseVariableItem *variableItem(ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    QRectF propertiesRect() const;

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    ProjectScene *mScene;
    QRectF mBounds;
    BaseNode *mNode;
    QList<BaseVariableItem*> mVariableItems;
    QList<NodeInputItem*> mInputItems;
    QList<NodeOutputItem*> mOutputItems;
    QPointF mPreDragPosition;
};

#endif // DRAFTOBJECTITEM_H
