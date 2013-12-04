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

#include "luamanager.h"
#include "metaeventmanager.h"
#include "node.h"
#include "project.h"
#include "projectactions.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "scriptmanager.h"
#include "scriptscene.h"
#include "scriptvariable.h"

#include <QDrag>
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
    mOutputsItem(new NodeOutputGroupItem(scene, node, this)),
    mOpenImage(QLatin1String(":/images/16x16/document-open.png")),
    mDeleteImage(QLatin1String(":/images/16x16/edit-delete.png"))
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsScenePositionChanges, true);
    setAcceptHoverEvents(true);
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
    QColor color = Qt::black;
    if (LuaNode *lnode = mNode->asLuaNode())
        if (!lnode->mInfo || !lnode->mInfo->node())
            color = Qt::red;
    if (MetaEventNode *enode = mNode->asEventNode())
        if (!enode->info() || !enode->info()->node())
            color = Qt::red;
    if (ScriptNode *snode = mNode->asScriptNode())
        if (!snode->info() || !snode->info()->node())
            color = Qt::red;

    QPen pen(color);
    pen.setCosmetic(true);
    painter->setPen(pen);

    QColor bg(238, 238, 255);
    if (mNode->isEventNode())
        bg = QColor(255, 255, 155);
    QRect textRect = painter->fontMetrics().boundingRect(mNode->label());
    QRectF nameRect = QRectF(mBounds.x(), mBounds.y(), textRect.width() + 6, textRect.height() + 6);
    painter->fillRect(nameRect, bg);
    painter->drawRect(nameRect);
    painter->drawText(nameRect, Qt::AlignCenter, mNode->label());

    QRectF bodyRect = mBounds.adjusted(0, nameRect.height(), 0, 0);
    painter->fillRect(bodyRect, bg);
    painter->drawRect(bodyRect);

    if (option->state & QStyle::State_MouseOver) {
        QRectF r = deleteRect();
        painter->fillRect(r, bg);
//        painter->drawRect(r);
        painter->drawImage(r.x() + (r.width() - mDeleteImage.width()) / 2,
                           r.y() + (r.height() - mDeleteImage.height()) / 2,
                           mDeleteImage);

        r = openRect();
        painter->fillRect(r, bg);
        painter->drawImage(r.x() + (r.width() - mOpenImage.width()) / 2,
                           r.y() + (r.height() - mOpenImage.height()) / 2,
                           mOpenImage);
    }
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
        mScene->document()->changer()->beginUndoCommand(mScene->document()->undoStack(), true);
        mScene->document()->changer()->doMoveNode(mNode, pos());
        mScene->document()->changer()->endUndoCommand();
    }

    if (openRect().contains(event->pos())) {
        if (ScriptNode *snode = mNode->asScriptNode())
            ProjectActions::instance()->openProject(snode->source());
    }
    if (deleteRect().contains(event->pos())) {
        ProjectActions::instance()->removeNode(mNode);
    }
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // FIXME: the mouseReleaseEvent doesn't get delivered due to this dialog.
    // BaseGraphicsView detects that and prints a message to the console.
    // Perhaps call this in mouseReleaseEvent().
    ProjectActions::instance()->nodePropertiesDialog(mNode);
}

void NodeItem::infoChanged(MetaEventInfo *info)
{
    if (MetaEventNode *node = mNode->asEventNode()) {
        if (node->info() == info) {
            if (node->syncWithInfo())
                syncWithNode();
            update();
        }
    }
}

void NodeItem::infoChanged(ScriptInfo *info)
{
    if (ScriptNode *node = mNode->asScriptNode()) {
        if (node->info() == info) {
            if (node->syncWithInfo())
                syncWithNode();
            update();
        }
    }
}

void NodeItem::infoChanged(LuaInfo *info)
{
    if (LuaNode *node = mNode->asLuaNode()) {
        if (node->mInfo == info) {
            if (node->syncWithInfo())
                syncWithNode();
            update();
        }
    }
}

void NodeItem::inputsChanged()
{
    mInputsItem->syncWithNode();
    updateLayout();
}

void NodeItem::outputsChanged()
{
    mOutputsItem->syncWithNode();
    updateLayout();
}

void NodeItem::variablesChanged()
{
    mVariablesItem->syncWithNode();
    updateLayout();
}

bool NodeItem::displaysVariable(ScriptVariable *var)
{
    return mVariablesItem->displaysVariable(var);
}

void NodeItem::updateLayout()
{
    prepareGeometryChange();

    mVariablesItem->updateLayout();
    mInputsItem->updateLayout();
    mOutputsItem->updateLayout();

    int inputsHeight = mInputsItem->childrenBoundingRect().height();
    int outputsHeight = mOutputsItem->childrenBoundingRect().height();
    QSize variablesSize = mVariablesItem->childrenBoundingRect().size().toSize();
    QSize nameSize = QFontMetrics(mScene->font()).boundingRect(mNode->label()).size() + QSize(6, 6);
    int buttonsWidth = 2 + qMax(nameSize.height(), 16) * 2 + 1;
    int variablesPadding = 8;

    mBounds = QRectF(0, 0, qMax(nameSize.width() + buttonsWidth, variablesSize.width() + variablesPadding * 2),
                     nameSize.height() + qMax(variablesSize.height() + variablesPadding * 2, qMax(inputsHeight, outputsHeight)));
    mBounds.translate(-mBounds.width() / 2, -mBounds.height() / 2);

    mVariablesItem->setPos(mBounds.x() + variablesPadding, mBounds.y() + nameSize.height() + variablesPadding);

    QRectF r = mBounds.adjusted(0, nameSize.height(), 0, 0);
    mInputsItem->setPos(r.left() - 1, r.center().y());
    mOutputsItem->setPos(r.right() + 1, r.center().y());
}

void NodeItem::syncWithNode()
{
    mVariablesItem->syncWithNode();
    mInputsItem->syncWithNode();
    mOutputsItem->syncWithNode();

    updateLayout();
}

QRectF NodeItem::nameRect()
{
    QRect textRect = QFontMetrics(mScene->font()).boundingRect(mNode->label());
    return QRectF(mBounds.x(), mBounds.y(), textRect.width() + 6, textRect.height() + 6);
}

QRectF NodeItem::deleteRect()
{
    QRectF r = nameRect();
    int d = qMax(qreal(16), r.height());
    return QRectF(mBounds.right() - d, mBounds.top(), d, r.height());
}

QRectF NodeItem::openRect()
{
    QRectF r = deleteRect();
    return r.adjusted(-r.width() - 1, 0, -r.width() - 1, 0);
}

/////

NodeInputItem::NodeInputItem(ScriptScene *scene, NodeInput *pin, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mInput(pin),
    mConnectHighlight(false)
{
    setAcceptHoverEvents(true);
    updateLayout();
}

QRectF NodeInputItem::boundingRect() const
{
    return mBounds;
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
    if (mInput->node() != mScene->document()->project()->rootNode() && !mInput->isKnown())
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
    painter->drawText(r.adjusted(size().width() / 4, 0, 0, 0), Qt::AlignCenter, mInput->label());
}

void NodeInputItem::updateLayout()
{
    QFontMetricsF fm(mScene->font());
    qreal labelWidth = fm.boundingRect(mInput->label()).width();
    QRectF r(-(size().width() + labelWidth), -size().height() / 2, size().width() + labelWidth, size().height());
    QRectF bounds = r.adjusted(-3, -3, 3, 3); // adjust for pen width
    if (bounds != mBounds) {
        prepareGeometryChange();
        mBounds = bounds;
    }
}

/////

NodeOutputItem::NodeOutputItem(ScriptScene *scene, NodeOutput *pin, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mOutput(pin)
{
    setAcceptHoverEvents(true);
    updateLayout();
}

QRectF NodeOutputItem::boundingRect() const
{
    return mBounds;
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
    if (mOutput->node() != mScene->document()->project()->rootNode() && !mOutput->isKnown())
        color = Qt::red;
    if (option->state & QStyle::State_MouseOver)
        color = color.lighter();
    painter->fillPath(path, color);

    painter->setRenderHint(QPainter::Antialiasing, true);
//    QPen pen(QColor(197, 224, 229, 128), 2);
    QPen pen(QColor(Qt::black), 2);
    painter->setPen(pen);
    painter->drawPath(path);

    if (mOutput->node()->isEventNode())
        return;

    painter->drawText(r, Qt::AlignCenter, mOutput->label());
}

void NodeOutputItem::updateLayout()
{
    QFontMetricsF fm(mScene->font());
    QString label = mOutput->label();
    if (mOutput->node()->isEventNode())
        label.clear();
    qreal labelWidth = fm.boundingRect(label).width();
    QRectF r(0, -size().height() / 2, size().width() + labelWidth, size().height());
    QRectF bounds = r.adjusted(-3, -3, 3, 3); // adjust for pen width
    if (bounds != mBounds) {
        prepareGeometryChange();
        mBounds = bounds;
    }
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
        NodeInputItem *item = itemFor(input);
        if (item == 0)
            item = new NodeInputItem(mScene, input, this);
        items += item;
    }
    foreach (NodeInputItem *item, mItems)
        if (!items.contains(item))
            unknowns += item;
#if 1
    qDeleteAll(unknowns);
    mItems = items;
#else
    mItems = items + unknowns;
#endif
}

void NodeInputGroupItem::updateLayout()
{
    int pinCount = mNode->inputCount();
    int pinHeight = NodeInputItem::size().height();
    int gap = 16;
    int totalHeight = pinCount * pinHeight + (pinCount - 1) * gap;

    int y = 0 - totalHeight / 2 + pinHeight / 2;
    foreach (NodeInputItem *item, mItems) {
        item->updateLayout();
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
#if 1
    qDeleteAll(unknowns);
    mItems = items;
#else
    mItems = items + unknowns;
#endif
}

void NodeOutputGroupItem::updateLayout()
{
    int pinCount = mNode->outputCount();
    int pinHeight = NodeOutputItem::size().height();
    int gap = 16;
    int totalHeight = pinCount * pinHeight + (pinCount - 1) * gap;

    int y = 0 - totalHeight / 2 + pinHeight / 2;
    foreach (NodeOutputItem *item, mItems) {
        item->updateLayout();
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
    mDropHighlight(false),
    mRemoveVarRefImage(QLatin1String(":/images/16x16/window-close.png"))
{
    setAcceptDrops(true);

    mGroupLabelWidth = mGroupValueWidth = 32;
}

QRectF BaseVariableItem::boundingRect() const
{
    return QRectF(0, 0, mGroupLabelWidth + mGroupValueWidth, 24);
}

void BaseVariableItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = Qt::black;
    if (!mVariable->isKnown())
        color = Qt::red;
    if (mDropHighlight)
        color = Qt::green;
    painter->setPen(color);
    painter->drawRect(valueRect(option->rect).adjusted(0,0,-1,-1));

    if (mVariable->node()->isEventNode()) {
        painter->drawText(valueRect(option->rect).adjusted(3,0,-3,0), Qt::AlignVCenter, mVariable->label());
        return;
    }

    painter->drawText(option->rect, Qt::AlignVCenter, mVariable->label());

    QRectF r = valueRect(option->rect).adjusted(3,0,-3,0);
    QString value = valueString();
    if (mVariable->variableRef().length()) {
        if (BaseNode *node = mScene->document()->project()->rootNode()->nodeByID(mVariable->variableRefID()))
            if (node->isEventNode())
                painter->fillRect(valueRect(option->rect).adjusted(1,1,-2,-2), QColor(255, 255, 155));

        QFont font = painter->font();
        font.setItalic(true);
        painter->setFont(font);
        painter->drawImage(clearRefRect(option->rect), mRemoveVarRefImage);
        painter->drawText(r, Qt::AlignVCenter, value);
    } else
        painter->drawText(r, Qt::AlignVCenter, value);
}

void BaseVariableItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMouseDownPos = event->pos();
    mWaitForDrag = false;
    if (event->button() == Qt::LeftButton) {
        if (clearRefRect(boundingRect()).contains(event->pos())) {
            return;
        }
        if (valueRect(boundingRect()).contains(event->pos())) {
            mWaitForDrag = true;
            return; // avoid calling QGraphicsItem::mousePressEvent() so
                    // the NodeItem isn't dragged
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

static QString VARIABLE_MIME_TYPE = QLatin1String("application/x-pzdraft-variable");

void BaseVariableItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mWaitForDrag) {
        if ((event->pos() - mMouseDownPos).manhattanLength()
                < QApplication::startDragDistance())
            return;

        QByteArray ba;
        QDataStream ds(&ba, QIODevice::WriteOnly);
        if (mVariable->variableRef().length())
            ds << QString::fromLatin1("%1:%2").arg(mVariable->variableRefID()).arg(mVariable->variableRef());
        else
            ds << QString::fromLatin1("%1:%2").arg(mVariable->node()->id()).arg(mVariable->name());

        QMimeData *mimeData = new QMimeData;
        mimeData->setData(VARIABLE_MIME_TYPE, ba);

        QDrag drag(event->widget());
        drag.setMimeData(mimeData);

        // Create pixmap for the drag feedback
        QRect r = valueRect(boundingRect()).toAlignedRect();
        QRect pixmapR = r.translated(-r.topLeft());
        QPixmap pixmap(r.width(), r.height());
        QColor bg(238, 238, 255);
        ScriptVariable *var = mVariable;
        if (mVariable->variableRef().length())
            var = referencedVariable();
        if (var && var->node()->isEventNode())
            bg = QColor(255, 255, 155);
        pixmap.fill(bg);
        QPainter painter(&pixmap);
        QFont font = mScene->font();
        painter.drawRect(pixmapR.adjusted(0,0,-1,-1));
        QString value = valueString();
        if (mVariable->variableRef().length())
            font.setItalic(true);
        painter.setFont(font);
        painter.drawText(pixmapR.adjusted(3,0,-3,0), Qt::AlignVCenter, value);
        painter.end();
        drag.setPixmap(pixmap);
        drag.setHotSpot((event->pos() - r.topLeft()).toPoint());

        drag.exec(Qt::LinkAction);
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void BaseVariableItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (clearRefRect(boundingRect()).contains(event->pos()) &&
                mVariable->variableRef().length()) {
            ProjectDocument *doc = mScene->document();
            doc->changer()->beginUndoCommand(doc->undoStack());
            doc->changer()->doSetVariableRef(mVariable, -1, QString());
            doc->changer()->endUndoCommand();
            return;
        }
        if (valueRect(boundingRect()).contains(event->pos()) &&
                mVariable->variableRef().isEmpty()) {
            ProjectActions::instance()->editNodeVariableValue(mVariable);
            return;
        }
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void BaseVariableItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (mVariable->node()->isEventNode()) {
        event->ignore();
        return;
    }

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
    Q_UNUSED(event)
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
                    if (var->node() == doc->project()->rootNode() ||
                            var->node()->isEventNode())
                        doc->changer()->doSetVariableRef(mVariable, var->node()->id(), var->name());
                    else
                        doc->changer()->doSetVariableValue(mVariable, var->value());
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

void BaseVariableItem::updateLayout(int groupLabelWidth, int groupValueWidth)
{
    if (groupLabelWidth != mGroupLabelWidth || mGroupValueWidth != groupValueWidth) {
        prepareGeometryChange();
        mGroupLabelWidth = groupLabelWidth;
        mGroupValueWidth = groupValueWidth;
    }
}

QSize BaseVariableItem::labelSizeHint()
{
    if (mVariable->node()->isEventNode())
        return QSize(0, 0);
    QFontMetrics fm(mScene->font());
    return fm.boundingRect(mVariable->label()).size() + QSize(4, 3 + 3);
}

QSize BaseVariableItem::valueSizeHint()
{
    QFontMetrics fm(mScene->font());
    QString value = valueString();
    bool xbox = true;
    if (mVariable->node()->isEventNode())
        xbox = false;
    return fm.boundingRect(value).size() + QSize(3 + 3 + (xbox ? 16 + 2 : 0), 3 + 3);
}

QRectF BaseVariableItem::valueRect(const QRectF &itemRect)
{
    return itemRect.adjusted(mGroupLabelWidth,0,0,0);
}

QRectF BaseVariableItem::clearRefRect(const QRectF &itemRect)
{
    return QRectF(itemRect.right() - 16 - 2,
                  itemRect.top() + (itemRect.height() - mRemoveVarRefImage.height()) / 2,
                  mRemoveVarRefImage.width(), mRemoveVarRefImage.height());
}

ScriptVariable *BaseVariableItem::referencedVariable()
{
    if (mVariable->variableRef().length()) {
        if (BaseNode *node = mScene->document()->project()->rootNode()->nodeByID(mVariable->variableRefID()))
            if (ScriptVariable *var = node->variable(mVariable->variableRef()))
                return var;
    }
    return NULL;
}

QString BaseVariableItem::valueString()
{
    if (mVariable->node()->isEventNode())
        return mVariable->label();
    QString value = mVariable->value();
    if (mVariable->variableRef().length()) {
        value = mVariable->variableRef();
        if (ScriptVariable *var = referencedVariable())
            value = var->label();
    }
    return value;
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
    int maxLabelWidth = 0, maxValueWidth = 96;
    foreach (BaseVariableItem *item, mItems) {
        maxLabelWidth = qMax(maxLabelWidth, item->labelSizeHint().width());
        maxValueWidth = qMax(maxValueWidth, item->valueSizeHint().width());
    }

    int gap = 4;
    int y = 0;
    foreach (BaseVariableItem *item, mItems) {
        item->updateLayout(maxLabelWidth, maxValueWidth);
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
#if 1
    qDeleteAll(unknowns);
    mItems = items;
#else
    mItems = items + unknowns;
#endif
}

bool VariableGroupItem::displaysVariable(ScriptVariable *var)
{
    foreach (BaseVariableItem *item, mItems) {
        if (item->mVariable == var) return true;
        if (item->referencedVariable() == var) return true;
    }
    return false;
}
