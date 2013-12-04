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

    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    static QSize size() { return QSize(22, 28); }

    void updateLayout();

    ScriptScene *mScene;
    NodeInput *mInput;
    QRectF mBounds;
    bool mConnectHighlight;
};

class NodeOutputItem : public QGraphicsItem
{
public:
    NodeOutputItem(ScriptScene *scene, NodeOutput *pin, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    static QSize size() { return QSize(22, 28); }

    void updateLayout();

    ScriptScene *mScene;
    NodeOutput *mOutput;
    QRectF mBounds;
    bool mConnectHighlight;
};

class NodeInputGroupItem : public QGraphicsItem
{
public:
    NodeInputGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    void syncWithNode();
    void updateLayout();

    void added(int index);
    void removed(int index);
    NodeInputItem *itemFor(const QString &name);
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

    void syncWithNode();
    void updateLayout();

    void added(int index);
    void removed(int index);
    NodeOutputItem *itemFor(const QString &name);
    NodeOutputItem *itemFor(NodeOutput *output);

    ScriptScene *mScene;
    BaseNode *mNode;
    QList<NodeOutputItem*> mItems;
};

class BaseVariableItem : public QGraphicsItem
{
public:
    BaseVariableItem(ScriptScene *scene, ScriptVariable *var, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

    QStringList getDropData(QGraphicsSceneDragDropEvent *event);

    void updateLayout(int groupLabelWidth, int groupValueWidth);
    QSize labelSizeHint();
    QSize valueSizeHint();

    QRectF valueRect(const QRectF &itemRect);
    QRectF clearRefRect(const QRectF &itemRect);

    ScriptVariable *referencedVariable();
    QString valueString();

    ScriptScene *mScene;
    ScriptVariable *mVariable;
    bool mDropHighlight;
    QPointF mMouseDownPos;
    bool mWaitForDrag;
    QImage mRemoveVarRefImage;

    int mGroupLabelWidth;
    int mGroupValueWidth;
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

class VariableGroupItem : public QGraphicsItem
{
public:
    VariableGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent = 0);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}

    BaseVariableItem *itemFor(const QString &name);
    BaseVariableItem *itemFor(ScriptVariable *var);
    void updateLayout();
    void syncWithNode();

    bool displaysVariable(ScriptVariable *var);

    ScriptScene *mScene;
    BaseNode *mNode;
    QList<BaseVariableItem*> mItems;
};

class NodeItem : public QGraphicsItem
{
public:
    NodeItem(ProjectScene *scene, BaseNode *node, QGraphicsItem *parent = 0);

    BaseNode *node() const { return mNode; }
    NodeInputItem *inputItem(NodeInput *input);
    NodeOutputItem *outputItem(NodeOutput *output);
    BaseVariableItem *variableItem(ScriptVariable *var);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void infoChanged(MetaEventInfo *info);
    void infoChanged(ScriptInfo *info);
    void infoChanged(LuaInfo *info);
    void inputsChanged();
    void outputsChanged();
    void variablesChanged();
    bool displaysVariable(ScriptVariable *var);
    void updateLayout();
    void syncWithNode();

    QRectF nameRect();
    QRectF deleteRect();
    QRectF openRect();

    ProjectScene *mScene;
    QRectF mBounds;
    BaseNode *mNode;
    VariableGroupItem *mVariablesItem;
    NodeInputGroupItem *mInputsItem;
    NodeOutputGroupItem *mOutputsItem;
    QPointF mPreDragPosition;
    QImage mOpenImage;
    QImage mDeleteImage;
};

#endif // DRAFTOBJECTITEM_H
