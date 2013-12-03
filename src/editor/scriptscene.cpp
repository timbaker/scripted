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

#include "scriptscene.h"

#include "luamanager.h"
#include "metaeventmanager.h"
#include "node.h"
#include "nodeitem.h"
#include "project.h"
#include "projectchanger.h"
#include "projectdocument.h"
#include "scriptmanager.h"

#include <QApplication>
#include <QFileInfo>
#include <QGraphicsSceneDragDropEvent>
#include <QStyleOptionGraphicsItem>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <QtMath>

ScriptScene::ScriptScene(ProjectDocument *doc, QObject *parent) :
    BaseGraphicsScene(ProjectSceneType, parent),
    mDocument(doc),
    mConnectionsItem(new ConnectionsItem(this)),
    mConnectFrom(0),
    mConnectTo(0),
    mGridItem(new GridItem(this)),
    mAreaItem(new ScriptAreaItem(this))
{
//    setBackgroundBrush(QColor(55, 74, 78));
    setBackgroundBrush(Qt::darkGray);

    addItem(mGridItem);
    addItem(mConnectionsItem);
    addItem(mAreaItem);

#if 1
    foreach (BaseNode *node, doc->project()->rootNode()->nodes()) {
        mNodeItems += createItemForNode(node);
    }
#elif 0
    if (DraftDefinition *dt = new DraftDefinition(tr("CheckInventoryItem"))) {
        dt->mProperties += new DraftProperty(tr("Entity"), QString(), QString());
        dt->mProperties += new DraftProperty(tr("Item"), QString(), QString());
        dt->mInputs += new DraftInput(tr("Invoke"), QStringList());
        dt->mInputs += new DraftInput(tr("SetEntity"), QStringList() << QLatin1String("string"));
        dt->mInputs += new DraftInput(tr("SetItem"), QStringList() << QLatin1String("string"));
        dt->mOutputs += new DraftOutput(tr("OnFound"), QStringList() << QLatin1String("string"));
        dt->mOutputs += new DraftOutput(tr("OnNotFound"), QStringList() << QLatin1String("string"));

        DraftObject *object = new DraftObject;
        object->mDefinition = dt;
        object->mPosition = QPointF(100, 100);
        object->mComment = tr("Here is a comment regarding this object.");

        createItemForObject(object);
    }

    if (DraftDefinition *dt = new DraftDefinition(tr("GiveItem"))) {
        dt->mProperties += new DraftProperty(tr("Entity"), QString(), QString());
        dt->mProperties += new DraftProperty(tr("Item"), QString(), QString());
        dt->mInputs += new DraftInput(tr("Invoke"), QStringList());
        dt->mInputs += new DraftInput(tr("SetEntity"), QStringList() << QLatin1String("string"));
        dt->mInputs += new DraftInput(tr("SetItem"), QStringList() << QLatin1String("string"));
        dt->mOutputs += new DraftOutput(tr("OnGiven"), QStringList() << QLatin1String("string"));

        DraftObject *object = new DraftObject;
        object->mDefinition = dt;
        object->mPosition = QPointF(400, 100);
        object->mComment = tr("Here is a comment regarding this object.");

        createItemForObject(object);
    }

#else
    FlowFragment *frag = new FlowFragment(this);
    frag->mName = tr("Burger Run");
    frag->mSynopsis = tr("The NPC group goes on a scouting mission for the mythical Spiffo's doll that protects against the plague.");
    frag->mInPins += new InputPin(frag);
    frag->mInPins += new InputPin(frag);
    frag->mOutPins += new OutputPin(frag);
    frag->mOutPins += new OutputPin(frag);
    frag->mOutPins += new OutputPin(frag);

    FlowFragmentItem *fragItem = new FlowFragmentItem(frag);
    fragItem->setPos(100, 100);
    addItem(fragItem);

    {
        int pinCount = frag->mInPins.size();
        int pinHeight = InputPinItem::size().height();
        int gap = 16;
        int height = pinCount * pinHeight + (pinCount - 1) * gap;
        int y = fragItem->boundingRect().center().y() - height / 2 + pinHeight / 2;
        for (int i = 0; i < pinCount; i++) {
            InputPin *pin = frag->mInPins[i];
            InputPinItem *pinItem = new InputPinItem(pin, fragItem);
            pinItem->setPos(fragItem->boundingRect().x(), y);
            y += pinHeight + gap;
        }
    }

    {
        int pinCount = frag->mOutPins.size();
        int pinHeight = OutputPinItem::size().height();
        int gap = 16;
        int height = pinCount * pinHeight + (pinCount - 1) * gap;
        int y = fragItem->boundingRect().center().y() - height / 2 + pinHeight / 2;
        for (int i = 0; i < pinCount; i++) {
            OutputPin *pin = frag->mOutPins[i];
            OutputPinItem *pinItem = new OutputPinItem(pin, fragItem);
            pinItem->setPos(fragItem->boundingRect().right(), y);
            y += pinHeight + gap;
        }
    }
#endif

    mAreaItem->updateBounds();

    connect(this, SIGNAL(sceneRectChanged(QRectF)), SLOT(sceneRectChanged()));

    connect(mDocument->changer(), SIGNAL(afterAddNode(int,BaseNode*)),
            SLOT(afterAddNode(int,BaseNode*)));
    connect(mDocument->changer(), SIGNAL(afterRemoveNode(int,BaseNode*)),
            SLOT(afterRemoveNode(int,BaseNode*)));
    connect(mDocument->changer(), SIGNAL(afterMoveNode(BaseNode*,QPointF)),
            SLOT(afterMoveNode(BaseNode*,QPointF)));
    connect(mDocument->changer(), SIGNAL(afterRenameNode(BaseNode*,QString)),
            SLOT(afterRenameNode(BaseNode*,QString)));

    connect(mDocument->changer(), SIGNAL(afterAddInput(int,NodeInput*)),
            SLOT(inputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterRemoveInput(int,NodeInput*)),
            SLOT(inputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterReorderInput(int,int)),
            SLOT(inputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterRenameInput(NodeInput*,QString)),
            SLOT(inputsChanged()));

    connect(mDocument->changer(), SIGNAL(afterAddOutput(int,NodeOutput*)),
            SLOT(outputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterRemoveOutput(int,NodeOutput*)),
            SLOT(outputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterReorderOutput(int,int)),
            SLOT(outputsChanged()));
    connect(mDocument->changer(), SIGNAL(afterRenameOutput(NodeOutput*,QString)),
            SLOT(outputsChanged()));

    connect(mDocument->changer(), SIGNAL(afterAddConnection(int,NodeConnection*)),
            SLOT(afterAddConnection(int,NodeConnection*)));
    connect(mDocument->changer(), SIGNAL(afterRemoveConnection(int,NodeConnection*)),
            SLOT(afterRemoveConnection(int,NodeConnection*)));
    connect(mDocument->changer(), SIGNAL(afterSetControlPoints(NodeConnection*,QPolygonF)),
            SLOT(afterSetControlPoints(NodeConnection*,QPolygonF)));

    connect(mDocument->changer(), SIGNAL(afterChangeVariable(ScriptVariable*,const ScriptVariable*)),
            SLOT(afterChangeVariable(ScriptVariable*,const ScriptVariable*)));

    connect(eventmgr(), SIGNAL(infoChanged(MetaEventInfo*)), SLOT(infoChanged(MetaEventInfo*)));
    connect(luamgr(), SIGNAL(infoChanged(LuaInfo*)),
            SLOT(infoChanged(LuaInfo*)));
    connect(scriptmgr(), SIGNAL(infoChanged(ScriptInfo*)),
            SLOT(infoChanged(ScriptInfo*)));

    mConnectionsItem->updateConnections();
}

void ScriptScene::setTool(AbstractTool *tool)
{
    Q_UNUSED(tool)
}

NodeItem *ScriptScene::createItemForNode(BaseNode *node)
{
    Q_ASSERT(itemForNode(node) == NULL);
    NodeItem *nodeItem = new NodeItem(this, node);
    nodeItem->setPos(node->pos());
    addItem(nodeItem);

    return nodeItem;
}

NodeItem *ScriptScene::itemForNode(BaseNode *node)
{
    foreach (NodeItem *item, mNodeItems) {
        if (item->node() == node)
            return item;
    }
    return 0;
}

QRectF ScriptScene::boundsOfAllNodes()
{
    QRectF r;

    foreach (NodeItem *item, mNodeItems) {
        QRectF r2 = item->mapRectToScene(item->boundingRect()) |
                item->mapRectToScene(item->childrenBoundingRect());
        if (r.isEmpty())
            r = r2;
        else
            r |= r2;
    }
    if (!r.isEmpty())
        r.adjust(-64, -64, 64, 64);
    return r;
}

void ScriptScene::moved(NodeInputItem *item)
{
    mConnectionsItem->moved(item);
}

void ScriptScene::moved(NodeOutputItem *item)
{
    mConnectionsItem->moved(item);
}

void ScriptScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mConnectTo) {
            if (mConnectTo) {
                mConnectTo->mConnectHighlight = false;
                mConnectTo->update();

                NodeConnection *cxn = new NodeConnection;
                cxn->mSender = mConnectFrom->mOutput->mNode;
                cxn->mOutput = mConnectFrom->mOutput->mName;
                cxn->mReceiver = mConnectTo->mInput->mNode;
                cxn->mInput = mConnectTo->mInput->mName;
                cxn->mControlPoints = mConnectionsItem->mNewConnectionPoints.mid(1, mConnectionsItem->mNewConnectionPoints.size() - 2);
                int index = cxn->mSender->connectionCount();

                mDocument->changer()->beginUndoCommand(mDocument->undoStack());
                mDocument->changer()->doAddConnection(index, cxn);
                mDocument->changer()->endUndoCommand();

                mConnectTo = 0;
            }
            mConnectionsItem->newConnectionEnd(mConnectTo);
            mConnectFrom = 0;
            return;
        }
        if (mConnectFrom) {
            mConnectionsItem->newConnectionClick(event->scenePos());
            return;
        }
        foreach (QGraphicsItem *item, items(event->scenePos())) {
            if (NodeOutputItem *outputItem = dynamic_cast<NodeOutputItem*>(item)) {
                mConnectFrom = outputItem;
                QPointF start(outputItem->mapToScene(outputItem->boundingRect().right() + 2,
                                                     outputItem->boundingRect().center().y()));
                mConnectionsItem->newConnectionStart(outputItem);
            }
        }
    }
    if (event->button() == Qt::RightButton) {
        if (mConnectFrom) {
            mConnectionsItem->newConnectionCancel();
            mConnectFrom = 0;
            mConnectTo = 0;
        }
        return;
    }

    BaseGraphicsScene::mousePressEvent(event);
}

void ScriptScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mConnectFrom) {
        mConnectionsItem->newConnectionHotspot(event->scenePos());

        NodeInputItem *highlight = 0;
        foreach (QGraphicsItem *item, items(event->scenePos())) {
            if (NodeInputItem *inputItem = dynamic_cast<NodeInputItem*>(item)) {
#if 1 // inputs can connect to themselves
                highlight = inputItem;
#else
                if (inputItem->mInput->mNode != mConnectFrom->mOutput->mNode) {
                    highlight = inputItem;
                }
#endif
                break;
            }
        }
        if (mConnectTo != highlight) {
            if (mConnectTo) {
                mConnectTo->mConnectHighlight = false;
                mConnectTo->update();
                mConnectTo = 0;
            }
            if (highlight) {
                highlight->mConnectHighlight = true;
                highlight->update();
                mConnectTo = highlight;
            }
        }
    }

    BaseGraphicsScene::mouseMoveEvent(event);
}

void ScriptScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    BaseGraphicsScene::mouseReleaseEvent(event);
}

static QString COMMAND_MIME_TYPE = QLatin1String("application/x-pzdraft-command");
static QString VARIABLE_MIME_TYPE = QLatin1String("application/x-pzdraft-variable");

void ScriptScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << event->mimeData()->formats();

    mDragHasPZS = false;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QFileInfo info(url.toLocalFile());
        if (!info.exists()) continue;
        if (!info.isFile()) continue;
        if (info.suffix() != QLatin1String("pzs")) continue;
        mDragHasPZS = true;
        break;
    }

    if (!event->mimeData()->hasFormat(COMMAND_MIME_TYPE) &&
            !event->mimeData()->hasFormat(VARIABLE_MIME_TYPE) &&
            !event->mimeData()->hasFormat(METAEVENT_MIME_TYPE) &&
            !mDragHasPZS) {
        event->ignore();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void ScriptScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);

    // NodeItem accepts VARIABLE_MIME_TYPE
    if (!event->isAccepted() &&
            (event->mimeData()->hasFormat(COMMAND_MIME_TYPE) ||
             event->mimeData()->hasFormat(METAEVENT_MIME_TYPE) ||
             mDragHasPZS)) {
        event->accept();
        event->setDropAction(Qt::CopyAction);
    }
}

void ScriptScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragLeaveEvent(event);
}

void ScriptScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(VARIABLE_MIME_TYPE)) {
        QGraphicsScene::dropEvent(event);
        return;
    }

    if (event->mimeData()->hasFormat(METAEVENT_MIME_TYPE)) {
        QByteArray encodedData = event->mimeData()->data(METAEVENT_MIME_TYPE);
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        while (!stream.atEnd()) {
            QString eventName;
            stream >> eventName;
            if (MetaEventInfo *info = eventmgr()->info(eventName)) {
                if (MetaEventNode *node = info->node()) { // may go to NULL if a MetaEvents.lua couldn't be reloaded
                    MetaEventNode *newNode = new MetaEventNode(mDocument->project()->mNextID++, node->name());
                    newNode->initFrom(node);
                    newNode->setPos(event->scenePos());
                    mDocument->changer()->beginUndoCommand(mDocument->undoStack());
                    mDocument->changer()->doAddNode(mDocument->project()->rootNode()->nodeCount(), newNode);
                    mDocument->changer()->endUndoCommand();
                }
            }
        }
        return;
    }

    if (mDragHasPZS) {
        foreach (const QUrl &url, event->mimeData()->urls()) {
            QFileInfo info(url.toLocalFile());
            if (!info.exists()) continue;
            if (!info.isFile()) continue;
            if (info.suffix() != QLatin1String("pzs")) continue;
            if (ScriptInfo *scriptInfo = scriptmgr()->scriptInfo(info.absoluteFilePath())) {
                ScriptNode *node = new ScriptNode(mDocument->project()->mNextID++, scriptInfo->node()->name());
                node->initFrom(scriptInfo->node());
                node->setPos(event->scenePos());
                node->setInfo(scriptInfo);
                node->setSource(scriptInfo->path());
                mDocument->changer()->beginUndoCommand(mDocument->undoStack());
                mDocument->changer()->doAddNode(mDocument->project()->rootNode()->nodeCount(), node);
                mDocument->changer()->endUndoCommand();
            }
        }
        return;
    }

    if (!event->mimeData()->hasFormat(COMMAND_MIME_TYPE))
        return;

    QByteArray encodedData = event->mimeData()->data(COMMAND_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        QString path;
        stream >> path;
        if (LuaInfo *def = luamgr()->luaInfo(path)) {
            if (LuaNode *lnode = def->node()) { // may go to NULL if a .lua file couldn't be reloaded
                LuaNode *node = new LuaNode(mDocument->project()->mNextID++, def->node()->name());
                node->initFrom(lnode);
                node->mInfo = def;
                node->setPos(event->scenePos());
                //            node->mComment = tr("Added by drag-and-drop.");
                mDocument->changer()->beginUndoCommand(mDocument->undoStack());
                mDocument->changer()->doAddNode(mDocument->project()->rootNode()->nodeCount(), node);
                mDocument->changer()->endUndoCommand();
            }
        }
    }
}

void ScriptScene::sceneRectChanged()
{
    mGridItem->updateBounds();
}

void ScriptScene::afterAddNode(int index, BaseNode *node)
{
    mNodeItems.insert(index, createItemForNode(node));
    mConnectionsItem->updateConnections();
    mAreaItem->updateBounds();
}

void ScriptScene::afterRemoveNode(int index, BaseNode *node)
{
    Q_UNUSED(node)
    delete mNodeItems.takeAt(index);
    mConnectionsItem->updateConnections();
    mAreaItem->updateBounds();
}

void ScriptScene::afterMoveNode(BaseNode *node, const QPointF &oldPos)
{
    Q_UNUSED(oldPos)
    if (NodeItem *item = itemForNode(node))
        item->setPos(node->pos());
    else
        Q_ASSERT(false);
    mAreaItem->updateBounds();
}

void ScriptScene::afterRenameNode(BaseNode *node, const QString &oldName)
{
    Q_UNUSED(oldName)
    if (NodeItem *item = itemForNode(node))
        item->updateLayout();
}

void ScriptScene::inputsChanged()
{
    mAreaItem->mInputsItem->syncWithNode();
    mAreaItem->mInputsItem->updateLayout();
}

void ScriptScene::outputsChanged()
{
    mAreaItem->mOutputsItem->syncWithNode();
    mAreaItem->mOutputsItem->updateLayout();
}

void ScriptScene::afterAddConnection(int index, NodeConnection *cxn)
{
    Q_UNUSED(index)
    Q_UNUSED(cxn)
    mConnectionsItem->updateConnections();
}

void ScriptScene::afterRemoveConnection(int index, NodeConnection *cxn)
{
    Q_UNUSED(index)
    Q_UNUSED(cxn)
    mConnectionsItem->updateConnections();
}

void ScriptScene::afterSetControlPoints(NodeConnection *cxn, const QPolygonF &oldPoints)
{
    mConnectionsItem->afterSetControlPoints(cxn);
}

void ScriptScene::afterChangeVariable(ScriptVariable *var, const ScriptVariable *oldValue)
{
    Q_UNUSED(oldValue)
    foreach (NodeItem *nodeItem, mNodeItems)
        if (BaseVariableItem *varItem = nodeItem->variableItem(var)) {
            nodeItem->updateLayout();
        }
}

void ScriptScene::infoChanged(MetaEventInfo *info)
{
    foreach (NodeItem *item, mNodeItems)
        item->infoChanged(info);

    mAreaItem->updateBounds();
}

void ScriptScene::infoChanged(ScriptInfo *info)
{
    foreach (NodeItem *item, mNodeItems)
        item->infoChanged(info);

    mAreaItem->updateBounds();
}

void ScriptScene::infoChanged(LuaInfo *info)
{
    foreach (NodeItem *item, mNodeItems)
        item->infoChanged(info);

    mAreaItem->updateBounds();
}

/////

ConnectionItem::ConnectionItem(NodeConnection *cxn, NodeOutputItem *outItem, NodeInputItem *inItem, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mConnection(cxn),
    mOutputItem(outItem),
    mInputItem(inItem),
    mShowNodes(false),
    mControlPointIndex(-1),
    mHighlightIndex(-1),
    mAddingNewPoint(false)
{
    setAcceptHoverEvents(true);
//    updateBounds();
}

void ConnectionItem::updateBounds()
{
    QPainterPath path;
    path.addPolygon(allPoints());
    QPainterPathStroker stroker;
    stroker.setWidth(NODE_RADIUS * 2);
    mShape = stroker.createStroke(path);

    QRectF bounds = mOutputItem->mapToScene(mOutputItem->boundingRect()).boundingRect() |
            mInputItem->mapToScene(mInputItem->boundingRect()).boundingRect();
    bounds |= controlPoints().boundingRect().adjusted(-NODE_RADIUS, -NODE_RADIUS, NODE_RADIUS, NODE_RADIUS);
    bounds.adjust(-6, -6, 6, 6);
    if (bounds != mBounds) {
        prepareGeometryChange();
        mBounds = bounds;
    }
}

QRectF ConnectionItem::boundingRect() const
{
    return mBounds;
}

void ConnectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(mShowNodes ?  QColor(128, 255, 255, 200) : QColor(255, 255, 255, 200), 2);
    painter->setPen(pen);
    QPointF start(mOutputItem->mapToScene(mOutputItem->boundingRect().right() + 2,
                                           mOutputItem->boundingRect().center().y()));
    QPointF end(mInputItem->mapToScene(mInputItem->boundingRect().left() - 2,
                                       mInputItem->boundingRect().center().y()));
    QPolygonF poly;
    poly << start << controlPoints();
    painter->drawPolyline(poly);

    int arrowSize = 6;
    QLineF line(poly.last(), end);
    line.setLength(line.length() - arrowSize);
    painter->drawLine(line);

    QPointF mid = line.pointAt((line.length() /*- arrowSize*/) / line.length());
    QPointF p1 = QLineF(mid, end).normalVector().p2();
    QLineF ll(p1, mid);
    ll.setLength(arrowSize * 2);
    QPointF p2 = ll.p2();
    QPainterPath path;
    path.moveTo(end);
    path.lineTo(p1);
    path.lineTo(p2);
    path.lineTo(end);
    painter->fillPath(path, pen.color());

    if (mShowNodes) {
        painter->setPen(Qt::NoPen);
//        painter->setBrush(pen.color());
        poly = controlPoints();
        for (int i = 0; i < poly.size(); i++) {
            painter->setBrush((i == mHighlightIndex) ? QColor(128, 255, 255, 200) : QColor(255, 255, 255, 200));
            painter->drawEllipse(poly[i], NODE_RADIUS, NODE_RADIUS);
        }
    }
}

QPainterPath ConnectionItem::shape() const
{
    return mShape;
}

void ConnectionItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    mShowNodes = true;
    update();
}

#include <QVector2D>

void ConnectionItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
#if 1
    int highlight = -1;
    for (int i = 0; i < mConnection->mControlPoints.size(); i++) {
        QPointF p = mConnection->mControlPoints[i];
        if (QLineF(event->scenePos(), p).length() <= NODE_RADIUS) {
            highlight = i;
            break;
        }
    }
    if (highlight != mHighlightIndex) {
        mHighlightIndex = highlight;
        qDebug() << mHighlightIndex;
        update();
    }
#else
    QPolygonF poly = allPoints;

    bool showNodes = false;
    int controlPointIndex = -1;
    for (int i = 0; i < poly.size() - 1; i++) {
        QVector2D dir = (QVector2D(poly[i+1]) - QVector2D(poly[i])).normalized();
        float d = QVector2D(event->scenePos()).distanceToLine(QVector2D(poly[i]), dir);
        if (d < NODE_RADIUS) {
            showNodes = true;
            break;
        }
    }
    if (showNodes != mShowNodes) {
        mShowNodes = showNodes;
        update();
    }
#endif
}

void ConnectionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mHighlightIndex = -1;
//    if (mShowNodes) {
        mShowNodes = false;
        update();
//    }
}

void ConnectionItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mControlPointIndex = -1;
        mMouseMoved = false;
        for (int i = 0; i < mConnection->mControlPoints.size(); i++) {
            QPointF p = mConnection->mControlPoints[i];
            if (QLineF(event->scenePos(), p).length() <= NODE_RADIUS) {
                mControlPointIndex = i;
                mControlPointDragPos = p;
                break;
            }
        }
        if (mControlPointIndex == -1 && mShowNodes) {
            // Click to add a new control point
            QPolygonF poly = allPoints();
            for (int i = 0; i < poly.size() - 1; i++) {
                QVector2D dir = (QVector2D(poly[i+1]) - QVector2D(poly[i])).normalized();
                float d = QVector2D(event->scenePos()).distanceToLine(QVector2D(poly[i]), dir);
                if (d < NODE_RADIUS) {
                    mAddingNewPoint = true;
                    mControlPointIndex = i; // not i + 1 'cuz start point isn't counted
                    mControlPointDragPos = event->scenePos();
                    updateBounds();
                    break;
                }
            }
        }
    }

//    QGraphicsItem::mousePressEvent(event);
}

void ConnectionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mControlPointIndex != -1) {
        if (!mMouseMoved && (event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength()
                >= QApplication::startDragDistance())
            mMouseMoved = true;
        if (mMouseMoved) {
            mControlPointDragPos = event->scenePos();
            updateBounds();
            update();
        }
    }
    //    QGraphicsItem::mouseMoveEvent(event);
}

void ConnectionItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mControlPointIndex != -1) {
            QPolygonF controlPoints = this->controlPoints();
            int index = mControlPointIndex;
            mControlPointIndex = -1;
            mAddingNewPoint = false;
            updateBounds();
            update();

            ScriptScene *scene = (ScriptScene*)this->scene();
            ProjectDocument *doc = scene->document();
            if (!mMouseMoved) {
                // delete point on click/release without movement
                doc->changer()->beginUndoCommand(doc->undoStack());
                controlPoints.remove(index);
                doc->changer()->doSetControlPoints(mConnection, controlPoints);
                doc->changer()->endUndoCommand();
                return;
            }
            doc->changer()->beginUndoCommand(doc->undoStack());
            doc->changer()->doSetControlPoints(mConnection, controlPoints);
            doc->changer()->endUndoCommand();
        }
    }

    // Unfortunately, mouse click events aren't reported to an item when
    // the left mouse button is already down
    if (event->button() == Qt::RightButton) {
        if (mControlPointIndex != -1) {
            mControlPointIndex = -1;
            mAddingNewPoint = false;
            updateBounds();
            update();
        }
    }
}

QPolygonF ConnectionItem::allPoints() const
{
    QPointF start(mOutputItem->mapToScene(mOutputItem->boundingRect().right() + 2,
                                           mOutputItem->boundingRect().center().y()));
    QPointF end(mInputItem->mapToScene(mInputItem->boundingRect().left() - 2,
                                       mInputItem->boundingRect().center().y()));
    QPolygonF ret;
    return ret << start << controlPoints() << end;
}

QPolygonF ConnectionItem::controlPoints() const
{
    QPolygonF ret = mConnection->mControlPoints;
    if (mAddingNewPoint)
        ret.insert(mControlPointIndex, mControlPointDragPos);
    else if (mControlPointIndex != -1)
        ret[mControlPointIndex] = mControlPointDragPos;
    return ret;
}

/////

ConnectionsItem::ConnectionsItem(ProjectScene *scene, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mNewConnectionItem(new QGraphicsPathItem)
{
    setFlag(ItemHasNoContents);

    QPen pen(QColor(255, 255, 255, 200), 2);
    mNewConnectionItem->setPen(pen);
}

void ConnectionsItem::updateConnections()
{
    qDeleteAll(mConnectionItems);
    mConnectionItems.clear();

    foreach (NodeItem *item, mScene->objectItems()) {
        foreach (NodeConnection *cxn, item->node()->connections()) {
            NodeOutput *output = item->node()->output(cxn->mOutput);
            NodeOutputItem *outputItem = output ? item->outputItem(output) : 0;
            NodeItem *receiverItem = mScene->itemForNode(cxn->mReceiver);
            NodeInput *input = receiverItem ? receiverItem->node()->input(cxn->mInput) : 0;
            NodeInputItem *inputItem = input ? receiverItem->inputItem(input) : 0;
            if (outputItem && inputItem) {
                mConnectionItems += new ConnectionItem(cxn, outputItem, inputItem, this);
                mConnectionItems.last()->updateBounds();
            }
        }
    }
}

ConnectionItem *ConnectionsItem::itemFor(NodeConnection *cxn)
{
    foreach (ConnectionItem *item, mConnectionItems)
        if (item->mConnection == cxn)
            return item;
    return 0;
}

void ConnectionsItem::moved(NodeInputItem *item)
{
    foreach (ConnectionItem *cxnItem, mConnectionItems) {
        if (cxnItem->mInputItem == item)
            cxnItem->updateBounds();
    }
}

void ConnectionsItem::moved(NodeOutputItem *item)
{
    foreach (ConnectionItem *cxnItem, mConnectionItems) {
        if (cxnItem->mOutputItem == item)
            cxnItem->updateBounds();
    }
}

void ConnectionsItem::newConnectionStart(NodeOutputItem *outputItem)
{
    mNewConnectionStart = outputItem;
    mNewConnectionPoints.clear();
    mNewConnectionPoints += outputItem->mapToScene(outputItem->boundingRect().right() + 2,
                                                   outputItem->boundingRect().center().y());
    mNewConnectionPoints += mNewConnectionPoints.last();

    QPainterPath path;
    path.addPolygon(mNewConnectionPoints);
    mNewConnectionItem->setPath(path);
    mScene->addItem(mNewConnectionItem);
}

void ConnectionsItem::newConnectionHotspot(const QPointF &scenePos)
{
    mNewConnectionPoints.last() = scenePos;
    QPainterPath path;
    path.addPolygon(mNewConnectionPoints);
    mNewConnectionItem->setPath(path);
}

void ConnectionsItem::newConnectionClick(const QPointF &scenePos)
{
    mNewConnectionPoints += scenePos;
    QPainterPath path;
    path.addPolygon(mNewConnectionPoints);
    mNewConnectionItem->setPath(path);
}

void ConnectionsItem::newConnectionEnd(NodeInputItem *inputItem)
{
    mScene->removeItem(mNewConnectionItem);
}

void ConnectionsItem::newConnectionCancel()
{
    mNewConnectionStart = 0;
    mScene->removeItem(mNewConnectionItem);
}

void ConnectionsItem::afterSetControlPoints(NodeConnection *cxn)
{
    if (ConnectionItem *item = itemFor(cxn)) {
        item->updateBounds();
        item->update();
    }
}

/////

GridItem::GridItem(ProjectScene *scene, QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    setFlag(ItemUsesExtendedStyleOption, true);
}

void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QPen pen(QColor(Qt::darkGray).darker(120));
    pen.setCosmetic(true);
    painter->setPen(pen);

    int GRID_SIZE = 38;
    int minX = qFloor(option->exposedRect.left() / GRID_SIZE) - 1;
    int maxX = qCeil(option->exposedRect.right() / GRID_SIZE) + 1;
    int minY = qFloor(option->exposedRect.top() / GRID_SIZE) - 1;
    int maxY = qCeil(option->exposedRect.bottom() / GRID_SIZE) + 1;
#if 0
    minX = qMax(0, minX);
    maxX = qMin(maxX, mWidth);
    minY = qMax(0, minY);
    maxY = qMin(maxY, mHeight);
#endif
    for (int x = minX; x <= maxX; x++)
        painter->drawLine(x * GRID_SIZE, minY * GRID_SIZE, x * GRID_SIZE, maxY * GRID_SIZE);

    for (int y = minY; y <= maxY; y++)
        painter->drawLine(minX * GRID_SIZE, y * GRID_SIZE, maxX * GRID_SIZE, y * GRID_SIZE);
}

void GridItem::updateBounds()
{
    if (mBounds != scene()->sceneRect()) {
        prepareGeometryChange();
        mBounds = scene()->sceneRect();
    }
}

/////

ScriptAreaItem::ScriptAreaItem(ProjectScene *scene, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mScene(scene),
    mInputsItem(new NodeInputGroupItem(scene, scene->document()->project()->rootNode(), this)),
    mOutputsItem(new NodeOutputGroupItem(scene, scene->document()->project()->rootNode(), this))
{

}

QRectF ScriptAreaItem::boundingRect() const
{
    return mBounds;
}

void ScriptAreaItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QBrush brush(QColor(255, 255, 255, 200), Qt::Dense4Pattern);
    QPen pen;
    pen.setBrush(brush);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawRect(mBounds);
}

void ScriptAreaItem::updateBounds()
{
    QRectF bounds = mScene->boundsOfAllNodes();
    if (bounds != mBounds) {
        prepareGeometryChange();
        mBounds = bounds;
        mInputsItem->setPos(mBounds.x() - 1, mBounds.center().y());
        mOutputsItem->setPos(mBounds.right() + 1, mBounds.center().y());
//        mInputsItem->updateLayout();
//        mOutputsItem->updateLayout();
    }
}
