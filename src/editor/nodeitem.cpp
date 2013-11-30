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

#include "nodeitem.h"

#include "scriptscene.h"
#include "node.h"
#include "project.h"
#include "projectactions.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "scriptvariable.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QScrollBar>
#include <QTableWidget>
#include <QPainter>

NodeItem::NodeItem(ProjectScene *scene, BaseNode *node, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNode(node),
    mVariablesItem(new VariableGroupItem(scene, node, this)),
    mInputsItem(new NodeInputGroupItem(scene, node, this)),
    mOutputsItem(new NodeOutputGroupItem(scene, node, this))
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsScenePositionChanges, true);
#if 0
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(4);
    setGraphicsEffect(effect);
#endif

    updateLayout();
}

NodeInputItem *NodeItem::inputItem(NodeInput *input)
{
    return mInputsItem->itemFor(input);
}

NodeOutputItem *NodeItem::outputItem(NodeOutput *output)
{
    return mOutputsItem->itemFor(output);
}

BaseVariableItem *NodeItem::variableItem(ScriptVariable *var)
{
    return mVariablesItem->itemFor(var);
}

QRectF NodeItem::boundingRect() const
{
    return mBounds;
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen(Qt::black);
    pen.setCosmetic(true);
    painter->setPen(pen);

    QColor bg(238, 238, 255);
    QRect textRect = painter->fontMetrics().boundingRect(mNode->name());
    QRectF nameRect = QRectF(mBounds.x(), mBounds.y(), textRect.width() + 6, textRect.height() + 6);
    painter->fillRect(nameRect, bg);
    painter->drawRect(nameRect);
    painter->drawText(nameRect, Qt::AlignCenter, mNode->name());

    QRectF bodyRect = mBounds.adjusted(0, nameRect.height(), 0, 0);
    painter->fillRect(bodyRect, bg);
    painter->drawRect(bodyRect);
}

QVariant NodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && scene()) {
        foreach (NodeInputItem *item, mInputsItem->mItems)
            mScene->moved(item);
        foreach (NodeOutputItem *item, mOutputsItem->mItems)
            mScene->moved(item);
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mPreDragPosition = pos();
    QGraphicsItem::mousePressEvent(event);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    if (mPreDragPosition != pos()) {
        mScene->document()->changer()->beginUndoCommand(mScene->document()->undoStack());
        mScene->document()->changer()->doMoveNode(mNode, pos());
        mScene->document()->changer()->endUndoCommand();

    }
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    ProjectActions::instance()->nodePropertiesDialog(mNode);
}

void NodeItem::scriptChanged(ScriptInfo *info)
{
    if (ScriptNode *node = mNode->asScriptNode()) {
        if (node->syncWithScriptInfo())
            syncWithNode();
    }
}

void NodeItem::updateLayout()
{
    prepareGeometryChange();

    int inputsHeight = mInputsItem->childrenBoundingRect().height();
    int outputsHeight = mOutputsItem->childrenBoundingRect().height();
    QSize variablesSize = mVariablesItem->childrenBoundingRect().size().toSize();
    QSize nameSize = QFontMetrics(mScene->font()).boundingRect(mNode->name()).size() + QSize(6, 6);
    int variablesPadding = 8;

    mBounds = QRectF(0, 0, qMax(nameSize.width(), variablesSize.width() + variablesPadding * 2),
                     nameSize.height() + qMax(variablesSize.height() + variablesPadding * 2, qMax(inputsHeight, outputsHeight)));

    mVariablesItem->setPos(variablesPadding, nameSize.height() + variablesPadding);

    QRectF r = mBounds.adjusted(0, nameSize.height(), 0, 0);
    mInputsItem->setPos(r.left() - 1, r.center().y());
    mOutputsItem->setPos(r.right() + 1, r.center().y());
}

void NodeItem::syncWithNode()
{
    mVariablesItem->syncWithNode();
    mVariablesItem->updateLayout();

    mInputsItem->syncWithNode();
    mInputsItem->updateLayout();

    mOutputsItem->syncWithNode();
    mOutputsItem->updateLayout();

    updateLayout();
}

/////

NodeInputItem::NodeInputItem(ScriptScene *scene, NodeInput *pin, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mInput(pin),
    mConnectHighlight(false)
{
    setAcceptHoverEvents(true);
}

QRectF NodeInputItem::boundingRect() const
{
    QFontMetricsF fm(mScene->font());
    qreal labelWidth = fm.boundingRect(mInput->mName).width();
    QRectF r(-(size().width() + labelWidth), -size().height() / 2, size().width() + labelWidth, size().height());
    return r.adjusted(-3, -3, 3, 3); // adjust for pen width
}

void NodeInputItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF r = boundingRect().adjusted(3, 3, -3, -3); // remove pen-width
    QPainterPath path;
    path.moveTo(r.topRight());
    path.lineTo(r.x() + size().width() * 0.65, r.top());
    path.arcTo(r.x(), r.y(), size().width() * 0.65, r.height(), 90, 180);
    path.lineTo(r.bottomRight());
    painter->setPen(Qt::NoPen);
//    QColor color = QColor(30, 40, 40);
    QColor color = Qt::gray;
    if (!mInput->isKnown())
        color = Qt::red;
    if (option->state & QStyle::State_MouseOver)
        color = color.lighter();
    painter->fillPath(path, color);

    painter->setRenderHint(QPainter::Antialiasing, true);
//    QPen pen(QColor(197, 224, 229, 128), 2);
    color = Qt::black;
    if (mConnectHighlight)
        color = QColor(Qt::green);
    QPen pen(color, 2);
    painter->setPen(pen);
    painter->drawPath(path);
    painter->drawText(r.adjusted(size().width() / 4, 0, 0, 0), Qt::AlignCenter, mInput->mName);
}

/////

NodeOutputItem::NodeOutputItem(ScriptScene *scene, NodeOutput *pin, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mOutput(pin)
{
    setAcceptHoverEvents(true);
}

QRectF NodeOutputItem::boundingRect() const
{
    QFontMetricsF fm(mScene->font());
    qreal labelWidth = fm.boundingRect(mOutput->mName).width();
    QRectF r(0, -size().height() / 2, size().width() + labelWidth, size().height());
    return r.adjusted(-3, -3, 3, 3); // adjust for pen width
}

void NodeOutputItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF r = boundingRect().adjusted(3, 3, -3, -3); // remove pen-width
    QPainterPath path;
    path.moveTo(r.topLeft());
    path.lineTo(r.right() - size().width() * 0.65, r.top());
    path.arcTo(r.right() - size().width() * 0.65, r.y(), size().width() * 0.65, r.height(), 90, -180);
    path.lineTo(r.bottomLeft());
    painter->setPen(Qt::NoPen);
//    QColor color = QColor(30, 40, 40);
    QColor color = Qt::gray;
    if (!mOutput->isKnown())
        color = Qt::red;
    if (option->state & QStyle::State_MouseOver)
        color = color.lighter();
    painter->fillPath(path, color);

    painter->setRenderHint(QPainter::Antialiasing, true);
//    QPen pen(QColor(197, 224, 229, 128), 2);
    QPen pen(QColor(Qt::black), 2);
    painter->setPen(pen);
    painter->drawPath(path);

    painter->drawText(r, Qt::AlignCenter, mOutput->mName);
}

/////

NodeInputGroupItem::NodeInputGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNode(node)
{
    setFlag(ItemHasNoContents);
    for (int i = 0; i < mNode->inputCount(); i++)
        added(i);
    updateLayout();
}

void NodeInputGroupItem::syncWithNode()
{
    QList<NodeInputItem*> items, unknowns;
    foreach (NodeInput *input, mNode->inputs()) {
        NodeInputItem *item = itemFor(input->name());
        if (item == 0)
            item = new NodeInputItem(mScene, input, this);
        items += item;
    }
    foreach (NodeInputItem *item, mItems)
        if (!items.contains(item))
            unknowns += item;
    mItems = items + unknowns;
}

void NodeInputGroupItem::updateLayout()
{
    int pinCount = mNode->inputCount();
    int pinHeight = NodeInputItem::size().height();
    int gap = 16;
    int totalHeight = pinCount * pinHeight + (pinCount - 1) * gap;

    int y = 0 - totalHeight / 2 + pinHeight / 2;
    foreach (NodeInputItem *item, mItems) {
        item->setPos(0, y);
        y += pinHeight + gap;
    }
}

void NodeInputGroupItem::added(int index)
{
    mItems.insert(index, new NodeInputItem(mScene, mNode->input(index), this));
    updateLayout();
}

void NodeInputGroupItem::removed(int index)
{
    delete mItems.takeAt(index);
    updateLayout();
}

NodeInputItem *NodeInputGroupItem::itemFor(const QString &name)
{
    foreach (NodeInputItem *item, mItems)
        if (item->mInput->name() == name)
            return item;
    return 0;
}

NodeInputItem *NodeInputGroupItem::itemFor(NodeInput *input)
{
    foreach (NodeInputItem *item, mItems)
        if (item->mInput == input)
            return item;
    return 0;
}

/////

NodeOutputGroupItem::NodeOutputGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNode(node)
{
    setFlag(ItemHasNoContents);
    for (int i = 0; i < mNode->outputCount(); i++)
        added(i);
    updateLayout();
}

void NodeOutputGroupItem::syncWithNode()
{
    QList<NodeOutputItem*> items, unknowns;
    foreach (NodeOutput *output, mNode->outputs()) {
        NodeOutputItem *item = itemFor(output->name());
        if (item == 0)
            item = new NodeOutputItem(mScene, output, this);
        items += item;
    }
    foreach (NodeOutputItem *item, mItems)
        if (!items.contains(item))
            unknowns += item;
    mItems = items + unknowns;
}

void NodeOutputGroupItem::updateLayout()
{
    int pinCount = mNode->outputCount();
    int pinHeight = NodeOutputItem::size().height();
    int gap = 16;
    int totalHeight = pinCount * pinHeight + (pinCount - 1) * gap;

    int y = 0 - totalHeight / 2 + pinHeight / 2;
    foreach (NodeOutputItem *item, mItems) {
        item->setPos(0, y);
        y += pinHeight + gap;
    }
}

void NodeOutputGroupItem::added(int index)
{
    mItems.insert(index, new NodeOutputItem(mScene, mNode->output(index), this));
    updateLayout();
}

void NodeOutputGroupItem::removed(int index)
{
    delete mItems.takeAt(index);
    updateLayout();
}

NodeOutputItem *NodeOutputGroupItem::itemFor(const QString &name)
{
    foreach (NodeOutputItem *item, mItems)
        if (item->mOutput->name() == name)
            return item;
    return 0;
}

NodeOutputItem *NodeOutputGroupItem::itemFor(NodeOutput *output)
{
    foreach (NodeOutputItem *item, mItems)
        if (item->mOutput == output)
            return item;
    return 0;
}

/////

BaseVariableItem::BaseVariableItem(ScriptScene *scene, ScriptVariable *var, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mVariable(var),
    mDropHighlight(false)
{
    setAcceptDrops(true);
}

QRectF BaseVariableItem::boundingRect() const
{
    return QRectF(0, 0, 200, 24);
}

void BaseVariableItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = Qt::black;
    if (!mVariable->isKnown())
        color = Qt::red;
    if (mDropHighlight)
        color = Qt::green;
    painter->setPen(color);
    painter->drawRect(option->rect.adjusted(100,0,0,0));
    painter->drawText(option->rect, Qt::AlignVCenter, mVariable->name());
    painter->drawText(option->rect.adjusted(100+3,0,-3,0), Qt::AlignVCenter, mVariable->value());
}

static QString VARIABLE_MIME_TYPE = QLatin1String("application/x-pzdraft-variable");

void BaseVariableItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(VARIABLE_MIME_TYPE)) {
        QStringList varNames = getDropData(event);
        bool accept = false;
        foreach (QString varName, varNames) {
            if (ScriptVariable *var = mScene->document()->project()->resolveVariable(varName)) {
                if (var->type() == mVariable->type()) {
                    accept = true;
                    break;
                }
            }
        }
        if (accept) {
            mDropHighlight = true;
            update();
            event->accept();
            return;
        }
    }
    event->ignore();
}

void BaseVariableItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (mDropHighlight) {
        mDropHighlight = false;
        update();
    }
}

void BaseVariableItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    ProjectDocument *doc = mScene->document();

    mDropHighlight = false;

    if (event->mimeData()->hasFormat(VARIABLE_MIME_TYPE)) {
        QStringList varNames = getDropData(event);
        foreach (QString varName, varNames) {
            if (ScriptVariable *var = doc->project()->resolveVariable(varName)) {
                if (var->type() == mVariable->type()) {
                    doc->changer()->beginUndoCommand(doc->undoStack());
                    doc->changer()->doSetVariableValue(mVariable, var->name());
                    doc->changer()->endUndoCommand();
                    return;
                }
            }
        }
    }
}

QStringList BaseVariableItem::getDropData(QGraphicsSceneDragDropEvent *event)
{
    QStringList ret;

    QByteArray encodedData = event->mimeData()->data(VARIABLE_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        QString varName;
        stream >> varName;
        ret += varName;
    }

    return ret;
}

/////

VariableGroupItem::VariableGroupItem(ScriptScene *scene, BaseNode *node, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNode(node)
{
    setFlag(ItemHasNoContents, true);
    syncWithNode();
    updateLayout();
}

BaseVariableItem *VariableGroupItem::itemFor(const QString &name)
{
    foreach (BaseVariableItem *item, mItems)
        if (item->mVariable->name() == name)
            return item;
    return 0;
}

BaseVariableItem *VariableGroupItem::itemFor(ScriptVariable *var)
{
    foreach (BaseVariableItem *item, mItems)
        if (item->mVariable == var)
            return item;
    return 0;
}

void VariableGroupItem::updateLayout()
{
    int gap = 4;
    int y = 0;
    foreach (BaseVariableItem *item, mItems) {
        item->setPos(0, y);
        y += item->boundingRect().height() + gap;
    }
}

void VariableGroupItem::syncWithNode()
{
    QList<BaseVariableItem*> items, unknowns;
    foreach (ScriptVariable *var, mNode->variables()) {
        // FIXME: check for changing type
        BaseVariableItem *item = itemFor(var->name());
        if (item == 0)
            item = new BaseVariableItem(mScene, var, this);
        items += item;
    }
    foreach (BaseVariableItem *item, mItems)
        if (!items.contains(item))
            unknowns += item;
    mItems = items + unknowns;
}
