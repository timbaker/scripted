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

NodeItem::NodeItem(ProjectScene *scene, BaseNode *obj, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNode(obj)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsScenePositionChanges, true);
#if 0
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(4);
    setGraphicsEffect(effect);
#endif

    int inputsHeight = 0;
    if (true) {
        int pinCount = mNode->inputCount();
        int pinHeight = NodeInputItem::size().height();
        int gap = 16;
        inputsHeight = pinCount * pinHeight + (pinCount - 1) * gap;
        for (int i = 0; i < pinCount; i++) {
            NodeInput *input = mNode->input(i);
            NodeInputItem *inputItem = new NodeInputItem(scene, input, this);
            mInputItems += inputItem;
        }
    }

    int outputsHeight = 0;
    if (true) {
        int pinCount = mNode->outputCount();
        int pinHeight = NodeOutputItem::size().height();
        int gap = 16;
        outputsHeight = pinCount * pinHeight + (pinCount - 1) * gap;
        for (int i = 0; i < pinCount; i++) {
            NodeOutput *output = mNode->output(i);
            NodeOutputItem *outputItem = new NodeOutputItem(scene, output, this);
            mOutputItems += outputItem;
        }
    }

    QSize variablesSize;
    if (mNode->variableCount()) {
        int maxWidth = 0, totalHeight = 0;
        int gap = 4;
        foreach (ScriptVariable *var, mNode->variables()) {
            BaseVariableItem *item = new BaseVariableItem(this, var);
            totalHeight += item->boundingRect().height() + gap;
            maxWidth = qMax(maxWidth, 200);
            mVariableItems += item;
        }
        totalHeight -= gap;
        variablesSize = QSize(maxWidth, totalHeight);
    }

    QSize nameSize = QFontMetrics(mScene->font()).boundingRect(mNode->name()).size() + QSize(6, 6);

    int variablesPadding = 8;
    mBounds = QRectF(0, 0, qMax(nameSize.width(), variablesSize.width() + variablesPadding * 2),
                     nameSize.height() + qMax(variablesSize.height() + variablesPadding * 2, qMax(inputsHeight, outputsHeight)));

    {
        int x = variablesPadding;
        int y = mBounds.top() + nameSize.height() + variablesPadding;
        int gap = 4;
        foreach (BaseVariableItem *item, mVariableItems) {
            item->setPos(x, y);
            y += item->boundingRect().height() + gap;
        }
    }

    {
        int pinHeight = NodeInputItem::size().height();
        int gap = 16;
        int y = mBounds.adjusted(0,nameSize.height(),0,0).center().y() - inputsHeight / 2 + pinHeight / 2;
        foreach (NodeInputItem *inputItem, mInputItems) {
            inputItem->setPos(mBounds.left() - 1, y);
            y += pinHeight + gap;
        }
    }

    {
        int pinHeight = NodeOutputItem::size().height();
        int gap = 16;
        int y = mBounds.adjusted(0,nameSize.height(),0,0).center().y() - outputsHeight / 2 + pinHeight / 2;
        foreach (NodeOutputItem *outputItem, mOutputItems) {
            outputItem->setPos(mBounds.right() + 1, y);
            y += pinHeight + gap;
        }
    }

#if 0
    QTableWidget *table = new QTableWidget(mNode->variableCount(), 2);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    table->setFrameShape(QFrame::NoFrame);
    table->setShowGrid(false);
    table->setStyleSheet(QLatin1String("QTableWidget {background-color: transparent; alternate-background-color: rgba(128,255,255,32); color: white; selection-color: white;}"));
#if 1
    static const char *ss = "     QScrollBar:vertical  { \
            border: 1px solid blue;\
            background: #32CC99;\
            width: 15px;\
            margin: 22px 0 22px 0;\
        }\
        QScrollBar::handle:vertical  {\
            background: white;\
            min-height: 20px;\
        }\
        QScrollBar::add-line:vertical  {\
            border: 2px solid grey;\
            background: #32CC99;\
            height: 20px;\
            subcontrol-position: bottom;\
            subcontrol-origin: margin;\
        }\
        QScrollBar::sub-line:vertical  {\
            border: 2px solid grey;\
            background: #32CC99;\
            height: 20px;\
            subcontrol-position: top;\
            subcontrol-origin: margin;\
        }\
        QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical  {\
            border: 2px solid grey;\
            width: 3px;\
            height: 3px;\
            background: white;\
        }\
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical  {\
            background: none;\
        }";
#endif
    table->verticalScrollBar()->setStyleSheet(QLatin1String(ss));
    table->setAlternatingRowColors(true);
    QRectF r = propertiesRect();
    table->setGeometry(r.toRect().adjusted(3, 3, -3, -3));
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(table);

    int row = 0;
    foreach (ScriptVariable *v, mNode->variables()) {
        QTableWidgetItem *item0 = new QTableWidgetItem;
        item0->setText(v->type());
        QTableWidgetItem *item1 = new QTableWidgetItem;
        item1->setText(v->name());
        table->setItem(row, 0, item0);
        table->setItem(row, 1, item1);
        row += 1;
    }
#endif
}

NodeInputItem *NodeItem::inputItem(NodeInput *input)
{
    foreach (NodeInputItem *item, mInputItems)
        if (item->mInput == input)
            return item;
    return NULL;
}

NodeOutputItem *NodeItem::outputItem(NodeOutput *output)
{
    foreach (NodeOutputItem *item, mOutputItems)
        if (item->mOutput == output)
            return item;
    return NULL;
}

BaseVariableItem *NodeItem::variableItem(ScriptVariable *var)
{
    foreach (BaseVariableItem *item, mVariableItems)
        if (item->mVariable == var)
            return item;
    return NULL;
}

QRectF NodeItem::boundingRect() const
{
    return mBounds;
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
#if 1
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

#else
    // Background
    QRectF r = boundingRect();
    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    grad.setColorAt(0, QColor(48, 96, 104));
    grad.setColorAt(1, QColor(197, 224, 229));
    QBrush brush(grad);
    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(r, 8, 8);

    // Outer highlight
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(197, 224, 229, 128));
    painter->drawRoundedRect(r.adjusted(1,1,-1,-1), 8, 8);

    // Property list on top
    QRectF refRect = QRectF(r.x() + 12, r.top() + 36, r.width() - 24, (r.height() - 36 - 6 - 12) * 2 / 3.0);
    QLinearGradient grad2(refRect.topLeft(), refRect.bottomLeft());
    grad2.setColorAt(0, QColor(32, 55, 59));
    grad2.setColorAt(1, QColor(63, 82, 86));
    QBrush brush2(grad2);
    painter->setBrush(brush2);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(refRect, 5, 5);

    // Comment area on bottom
    float y = refRect.bottom() + 6;
    QRectF textRect = QRectF(refRect.x(), y, refRect.width(), r.bottom() - 12 - y);
    QLinearGradient grad3(textRect.topLeft(), textRect.bottomLeft());
    grad3.setColorAt(0, QColor(67, 85, 89));
    grad3.setColorAt(1, QColor(98, 112, 115));
    QBrush brush3(grad3);
    painter->setPen(Qt::NoPen);
    painter->fillRect(textRect, brush3);

    // Comment area's drop shadow
    QLinearGradient grad4(textRect.x(), textRect.y(), textRect.x(), textRect.y() + 12);
    grad4.setColorAt(0, QColor(48, 61, 62, 128));
    grad4.setColorAt(1, QColor(67, 85, 89, 128));
    painter->fillRect(textRect.x(), textRect.y(), textRect.width(), 12, grad4);
#if 0
    grad4.setFinalStop(textRect.x() + 12, textRect.y());
    painter->fillRect(textRect.x(), textRect.y() + 12, 12, textRect.height() - 12, grad4);
    grad4.setFinalStop(textRect.x() + 12, textRect.y() + 12);
    painter->fillRect(textRect.x(), textRect.y(), 12, 12, grad4);
#endif

    // Title
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSizeF(font.pointSizeF() + 2);
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(r.x() + 48, r.y(), r.width() - 48, 36), Qt::AlignVCenter, mNode->mName);

    // Text
    font.setPointSizeF(font.pointSizeF() - 2);
    painter->setFont(font);
    painter->drawText(textRect.adjusted(12, 12, -12, -12), mNode->mComment);
#endif
}

QVariant NodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && scene()) {
        foreach (NodeInputItem *item, mInputItems)
            mScene->moved(item);
        foreach (NodeOutputItem *item, mOutputItems)
            mScene->moved(item);
    }
    return QGraphicsItem::itemChange(change, value);
}

QRectF NodeItem::propertiesRect() const
{
    QRectF r = boundingRect();
    return QRectF(r.x() + 12, r.top() + 36, r.width() - 24, (r.height() - 36 - 6 - 12) * 2 / 3.0);
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
    QFontMetricsF fm(scene()->font());
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
    if (option->state & QStyle::State_MouseOver)
        color = color.lighter(175);
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
    QFontMetricsF fm(scene()->font());
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
    if (option->state & QStyle::State_MouseOver)
        color = color.lighter(175);
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

/////

BaseVariableItem::BaseVariableItem(NodeItem *parent, ScriptVariable *var) :
    QGraphicsItem(parent),
    mParent(parent),
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
    if (mDropHighlight)
        painter->setPen(Qt::green);
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
            if (ScriptVariable *var = mParent->mScene->document()->project()->resolveVariable(varName)) {
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
    ProjectDocument *doc = mParent->mScene->document();

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

