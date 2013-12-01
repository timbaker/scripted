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

#include "projectchanger.h"

#include "node.h"
#include "project.h"

#include <QDebug>
#include <QPainter>
#include <QUndoStack>

#if defined(Q_OS_WIN) && (_MSC_VER >= 1600)
// Hmmmm.  libtiled.dll defines the MapRands class as so:
// class TILEDSHARED_EXPORT MapRands : public QVector<QVector<int> >
// Suddenly I'm getting a 'multiply-defined symbol' error.
// I found the solution here:
// http://www.archivum.info/qt-interest@trolltech.com/2005-12/00242/RE-Linker-Problem-while-using-QMap.html
template class __declspec(dllimport) QVector<QVector<int> >;
template class __declspec(dllimport) QMap<QString,QString>;
#endif

namespace ProjectChanges
{
#if 1
enum ChangeId
{
    ID_AddObject
};

// FIXME: why didn't ImageEd delete objects in destructors?

class AddNode : public ProjectChange
{
public:
    AddNode(ProjectChanger *changer, int index, BaseNode *object) :
        ProjectChange(changer),
        mIndex(index),
        mNode(object)
    {
    }

    void redo()
    {
        mChanger->project()->rootNode()->insertNode(mIndex, mNode);
        mChanger->afterAddNode(mIndex, mNode);
    }

    void undo()
    {
        mChanger->beforeRemoveNode(mIndex, mNode);
        mChanger->project()->rootNode()->removeNode(mIndex);
        mChanger->afterRemoveNode(mIndex, mNode);
    }

    QString text() const
    {
        return mChanger->tr("Add Node");
    }

    int mIndex;
    BaseNode *mNode;
};

class RemoveNode : public AddNode
{
public:
    RemoveNode(ProjectChanger *changer, int index, BaseNode *node) :
        AddNode(changer, index, node)
    { }
    void redo() { AddNode::undo(); }
    void undo() { AddNode::redo(); }
    QString text() const { return mChanger->tr("Remove Node"); }
};

class MoveNode : public ProjectChange
{
public:
    MoveNode(ProjectChanger *changer, BaseNode *node, const QPointF &pos) :
        ProjectChange(changer),
        mNode(node),
        mOldPos(node->pos()),
        mNewPos(pos)
    {
    }

    void redo()
    {
        mNode->setPos(mNewPos);
        mChanger->afterMoveNode(mNode, mOldPos);
    }

    void undo()
    {
        mNode->setPos(mOldPos);
        mChanger->afterMoveNode(mNode, mNewPos);
    }

    QString text() const
    {
        return mChanger->tr("Move Object");
    }

    BaseNode *mNode;
    QPointF mOldPos;
    QPointF mNewPos;
};

class AddConnection : public ProjectChange
{
public:
    AddConnection(ProjectChanger *changer, int index, NodeConnection *cxn) :
        ProjectChange(changer),
        mIndex(index),
        mConnection(cxn)
    {
    }

    void redo()
    {
        mConnection->mSender->insertConnection(mIndex, mConnection);
        mChanger->afterAddConnection(mIndex, mConnection);
    }

    void undo()
    {
        mChanger->beforeRemoveConnection(mIndex, mConnection);
        mConnection->mSender->removeConnection(mConnection);
        mChanger->afterRemoveConnection(mIndex, mConnection);
    }

    QString text() const
    {
        return mChanger->tr("Add Connection");
    }

    int mIndex;
    NodeConnection *mConnection;
};

class RemoveConnection : public AddConnection
{
public:
    RemoveConnection(ProjectChanger *changer, int index, NodeConnection *cxn) :
        AddConnection(changer, index, cxn)
    { }
    void redo() { AddConnection::undo(); }
    void undo() { AddConnection::redo(); }
    QString text() const { return mChanger->tr("Remove Connection"); }
};

class AddVariable : public ProjectChange
{
public:
    AddVariable(ProjectChanger *changer, int index, ScriptVariable *cxn) :
        ProjectChange(changer),
        mIndex(index),
        mVariable(cxn)
    {
    }

    void redo()
    {
        mChanger->project()->rootNode()->insertVariable(mIndex, mVariable);
        mChanger->afterAddVariable(mIndex, mVariable);
    }

    void undo()
    {
        mChanger->beforeRemoveVariable(mIndex, mVariable);
        mChanger->project()->rootNode()->removeVariable(mVariable);
        mChanger->afterRemoveVariable(mIndex, mVariable);
    }

    QString text() const
    {
        return mChanger->tr("Add Variable");
    }

    int mIndex;
    ScriptVariable *mVariable;
};

class RemoveVariable : public AddVariable
{
public:
    RemoveVariable(ProjectChanger *changer, int index, ScriptVariable *cxn) :
        AddVariable(changer, index, cxn)
    { }
    void redo() { AddVariable::undo(); }
    void undo() { AddVariable::redo(); }
    QString text() const { return mChanger->tr("Remove Variable"); }
};

class ChangeVariable : public ProjectChange
{
public:
    ChangeVariable(ProjectChanger *changer, ScriptVariable *var, const ScriptVariable &value) :
        ProjectChange(changer),
        mVariable(var),
        mNewValue(&value),
        mOldValue(var)
    {
    }

    void redo()
    {
        *mVariable = ScriptVariable(&mNewValue);
        mChanger->afterChangeVariable(mVariable, &mOldValue);
    }

    void undo()
    {
        *mVariable = ScriptVariable(&mOldValue);
        mChanger->afterChangeVariable(mVariable, &mNewValue);
    }

    QString text() const
    {
        return mChanger->tr("Change Variable");
    }

    ScriptVariable *mVariable;
    ScriptVariable mNewValue;
    ScriptVariable mOldValue;
};

#else
enum ChangeId
{
    ID_MoveNode = 1,
    ID_MoveNodes,
    ID_AddPath,
    ID_RemovePath,
    ID_ReorderPath,
    ID_AddNodeToPath,
    ID_RemoveNodeFromPath,
    ID_AddScriptToPath,
    ID_AddPathLayer,
    ID_RemovePathLayer,
    ID_ReorderPathLayer,
    ID_RenamePathLayer,
    ID_ChangeScriptParameters,
    ID_SetPathVisible,
    ID_SetPathClosed,
#if 0
    ID_SetPathTextureParams,
#endif
    ID_SetPathStroke,
    ID_SetPathColor,
    ID_SetPathLayerVisible,
    ID_AddImage,
    ID_RemoveImage,
    ID_MoveImage,
    ID_Paint
};

class MoveNode : public ProjectChange
{
public:
    MoveNode(ProjectChanger *changer, Node *node, const QPointF &pos) :
        ProjectChange(changer),
        mNode(node),
        mNewPos(pos),
        mOldPos(node->pos())
    {

    }

    void redo()
    {
        mNode->setPos(mNewPos);
        mChanger->afterMoveNode(mNode, mOldPos);
    }

    void undo()
    {
        mNode->setPos(mOldPos);
        mChanger->afterMoveNode(mNode, mNewPos);
    }

    QString text() const
    {
        return mChanger->tr("Move Node");
    }

    Node *mNode;
    QPointF mNewPos;
    QPointF mOldPos;
};

class MoveNodes : public ProjectChange
{
public:
    MoveNodes(ProjectChanger *changer, NodeList nodes, const QList<QPointF> &pos) :
        ProjectChange(changer),
        mNodes(nodes),
        mNewPos(pos)
    {
        foreach (Node *node, mNodes)
            mOldPos += node->pos();
    }

    void redo()
    {
        NodeList::const_iterator itNode = mNodes.constBegin();
        QList<QPointF>::const_iterator itPos = mNewPos.constBegin();
        while (itNode != mNodes.constEnd()) {
            (*itNode)->setPos(*itPos);
            itNode++, itPos++;
        }
        mChanger->afterMoveNodes(mNodes, mOldPos);
    }

    void undo()
    {
        NodeList::const_iterator itNode = mNodes.constBegin();
        QList<QPointF>::const_iterator itPos = mOldPos.constBegin();
        while (itNode != mNodes.constEnd()) {
            (*itNode)->setPos(*itPos);
            itNode++, itPos++;
        }
        mChanger->afterMoveNodes(mNodes, mNewPos);
    }

    QString text() const
    {
        return mChanger->tr("Move Nodes");
    }

    NodeList mNodes;
    QList<QPointF> mNewPos;
    QList<QPointF> mOldPos;
};

class AddPath : public ProjectChange
{
public:
    AddPath(ProjectChanger *changer, PathLayer *layer, int index, Path *path) :
        ProjectChange(changer),
        mLayer(layer),
        mIndex(index),
        mPath(path)
    {
    }

    void redo()
    {
        mLayer->insertPath(mIndex, mPath);
        mChanger->afterAddPath(mLayer, mIndex, mPath);
    }

    void undo()
    {
        mChanger->beforeRemovePath(mLayer, mIndex, mPath);
        mLayer->removePath(mIndex);
        mChanger->afterRemovePath(mLayer, mIndex, mPath);
    }

    QString text() const
    {
        return mChanger->tr("Add Path");
    }

    PathLayer *mLayer;
    int mIndex;
    Path *mPath;
};

class RemovePath : public AddPath
{
public:
    RemovePath(ProjectChanger *changer, PathLayer *layer, int index, Path *path) :
        AddPath(changer, layer, index, path)
    { }
    void redo() { AddPath::undo(); }
    void undo() { AddPath::redo(); }
    QString text() const { return mChanger->tr("Remove Path"); }
};

class ReorderPath : public ProjectChange
{
public:
    ReorderPath(ProjectChanger *changer, Path *path, int newIndex) :
        ProjectChange(changer),
        mLayer(path->layer()),
        mPath(path),
        mNewIndex(newIndex),
        mOldIndex(path->layer()->indexOf(path))
    {
    }

    void redo()
    {
        mLayer->removePath(mOldIndex);
        mLayer->insertPath(mNewIndex, mPath);
        mChanger->afterReorderPath(mPath, mOldIndex);
    }

    void undo()
    {
        mLayer->removePath(mNewIndex);
        mLayer->insertPath(mOldIndex, mPath);
        mChanger->afterReorderPath(mPath, mNewIndex);
    }

    QString text() const
    {
        return mChanger->tr((mNewIndex > mOldIndex) ? "Move Path Up" : "Move Path Down");
    }

    PathLayer *mLayer;
    Path *mPath;
    int mNewIndex;
    int mOldIndex;
};

class AddNodeToPath : public ProjectChange
{
public:
    AddNodeToPath(ProjectChanger *changer, Path *path, int index, Node *node) :
        ProjectChange(changer),
        mPath(path),
        mIndex(index),
        mNode(node)
    {

    }

    void redo()
    {
        mPath->insertNode(mIndex, mNode);
        mChanger->afterAddNodeToPath(mPath, mIndex, mNode);
    }

    void undo()
    {
        mChanger->beforeRemoveNodeFromPath(mPath, mIndex, mNode);
        mPath->removeNode(mIndex);
        mChanger->afterRemoveNodeFromPath(mPath, mIndex, mNode);
    }

    QString text() const
    {
        return mChanger->tr("Add Node To Path");
    }

    Path *mPath;
    int mIndex;
    Node *mNode;
};

class RemoveNodeFromPath : public AddNodeToPath
{
public:
    RemoveNodeFromPath(ProjectChanger *changer, Path *path, int index, Node *node) :
        AddNodeToPath(changer, path, index, node)
    {}
    void redo() { AddNodeToPath::undo(); }
    void undo() { AddNodeToPath::redo(); }
    QString text() const { return mChanger->tr("Remove Node From Path"); }
};

class AddScriptToPath : public ProjectChange
{
public:
    AddScriptToPath(ProjectChanger *changer, Path *path, int index, WorldScript *script) :
        ProjectChange(changer),
        mPath(path),
        mIndex(index),
        mScript(script)
    {

    }

    ~AddScriptToPath()
    {
//        delete mScript;
    }

    void redo()
    {
        mPath->insertScript(mIndex, mScript);
        mChanger->afterAddScriptToPath(mPath, mIndex, mScript);
        mScript = 0;
    }

    void undo()
    {
        mScript = mPath->removeScript(mIndex);
        mChanger->afterRemoveScriptFromPath(mPath, mIndex, mScript);
    }

    QString text() const
    {
        return mChanger->tr("Add Script To Path");
    }

    Path *mPath;
    int mIndex;
    WorldScript *mScript;
};

class AddLayer : public ProjectChange
{
public:
    AddLayer(ProjectChanger *changer, int index, Layer *layer) :
        ProjectChange(changer),
        mIndex(index),
        mLayer(layer)
    {
    }

    void redo()
    {
        mChanger->project()->insertLayer(mIndex, mLayer);
        mChanger->afterAddLayer(mIndex, mLayer);
    }

    void undo()
    {
        mChanger->beforeRemoveLayer(mIndex, mLayer);
        mChanger->project()->removeLayer(mIndex);
        mChanger->afterRemoveLayer(mIndex, mLayer);
    }

    QString text() const
    {
        return mChanger->tr("Add Layer");
    }

    int mIndex;
    Layer *mLayer;
};

class RemoveLayer : public AddLayer
{
public:
    RemoveLayer(ProjectChanger *changer, int index, Layer *layer) :
        AddLayer(changer, index, layer)
    { }
    void redo() { AddLayer::undo(); }
    void undo() { AddLayer::redo(); }
    QString text() const { return mChanger->tr("Remove Layer"); }
};

class ReorderLayer : public ProjectChange
{
public:
    ReorderLayer(ProjectChanger *changer, Layer *layer, int newIndex) :
        ProjectChange(changer),
        mNewIndex(newIndex),
        mOldIndex(changer->project()->indexOf(layer)),
        mLayer(layer)
    {
    }

    void redo()
    {
        mChanger->project()->removeLayer(mOldIndex);
        mChanger->project()->insertLayer(mNewIndex, mLayer);
        mChanger->afterReorderLayer(mLayer, mOldIndex);
    }

    void undo()
    {
        mChanger->project()->removeLayer(mNewIndex);
        mChanger->project()->insertLayer(mOldIndex, mLayer);
        mChanger->afterReorderLayer(mLayer, mNewIndex);
    }

    QString text() const
    {
        return mChanger->tr((mNewIndex > mOldIndex) ? "Move Layer Up" : "Move Layer Down");
    }

    int mNewIndex;
    int mOldIndex;
    Layer *mLayer;
};

class RenameLayer : public ProjectChange
{
public:
    RenameLayer(ProjectChanger *changer, Layer *layer, const QString &name) :
        ProjectChange(changer),
        mLayer(layer),
        mNewName(name),
        mOldName(layer->name())
    {
    }

    void redo()
    {
        mLayer->setName(mNewName);
        mChanger->afterRenameLayer(mLayer, mOldName);
    }

    void undo()
    {
        mLayer->setName(mOldName);
        mChanger->afterRenameLayer(mLayer, mNewName);
    }

    QString text() const
    {
        return mChanger->tr("Rename Layer");
    }

    Layer *mLayer;
    QString mNewName;
    QString mOldName;
};

class ChangeScriptParameters : public ProjectChange
{
public:
    ChangeScriptParameters(ProjectChanger *changer, WorldScript *script, const ScriptParams &params) :
        ProjectChange(changer),
        mScript(script),
        mNewParams(params),
        mOldParams(script->params())
    {

    }

    void redo()
    {
        mScript->setParams(mNewParams);
        mChanger->afterChangeScriptParameters(mScript);
    }

    void undo()
    {
        mScript->setParams(mOldParams);
        mChanger->afterChangeScriptParameters(mScript);
    }

    QString text() const
    {
        return mChanger->tr("Change Script Parameters");
    }

    WorldScript *mScript;
    ScriptParams mNewParams;
    ScriptParams mOldParams;
};

class SetPathVisible : public ProjectChange
{
public:
    SetPathVisible(ProjectChanger *changer, Path *path, bool visible) :
        ProjectChange(changer),
        mPath(path),
        mVisible(visible),
        mWasVisible(path->isVisible())
    {
    }

    void redo()
    {
        mPath->setVisible(mVisible);
        mChanger->afterSetPathVisible(mPath, mVisible);
    }

    void undo()
    {
        mPath->setVisible(mWasVisible);
        mChanger->afterSetPathVisible(mPath, mWasVisible);
    }

    QString text() const
    {
        return mChanger->tr(mVisible ? "Show Path" : "Hide Path");
    }

    Path *mPath;
    bool mVisible;
    bool mWasVisible;
};

class SetPathClosed : public ProjectChange
{
public:
    SetPathClosed(ProjectChanger *changer, Path *path, bool closed) :
        ProjectChange(changer),
        mPath(path),
        mClosed(closed),
        mWasClosed(path->isClosed())
    {
    }

    void redo()
    {
        mPath->setClosed(mClosed);
        mChanger->afterSetPathClosed(mPath, mClosed);
    }

    void undo()
    {
        mPath->setClosed(mWasClosed);
        mChanger->afterSetPathClosed(mPath, mWasClosed);
    }

    QString text() const
    {
        return mChanger->tr(mClosed ? "Close Path" : "Open Path");
    }

    Path *mPath;
    bool mClosed;
    bool mWasClosed;
};
#if 0
class SetPathTextureParams : public ProjectChange
{
public:
    enum Diff
    {
        DiffTexture = 0x01,
        DiffScaleX = 0x02,
        DiffScaleY = 0x04,
        DiffTranslateX = 0x08,
        DiffTranslateY = 0x10,
        DiffRotate = 0x20,
        DiffAlign = 0x40
    };

    SetPathTextureParams(ProjectChanger *changer, Path *path, const PathTexture &params) :
        ProjectChange(changer),
        mPath(path),
        mNewParams(params),
        mOldParams(path->texture()),
        mDiff(0)
    {
        if (mNewParams.mTexture != mOldParams.mTexture) mDiff |= DiffTexture;
        if (mNewParams.mScale.width() != mOldParams.mScale.width()) mDiff |= DiffScaleX;
        if (mNewParams.mScale.height() != mOldParams.mScale.height()) mDiff |= DiffScaleY;
        if (mNewParams.mTranslation.x() != mOldParams.mTranslation.x()) mDiff |= DiffTranslateX;
        if (mNewParams.mTranslation.y() != mOldParams.mTranslation.y()) mDiff |= DiffTranslateY;
        if (mNewParams.mRotation != mOldParams.mRotation) mDiff |= DiffRotate;
        if (mNewParams.mAlignWorld != mOldParams.mAlignWorld) mDiff |= DiffAlign;
    }

    void redo()
    {
        mPath->texture() = mNewParams;
        mChanger->afterSetPathTextureParams(mPath, mOldParams);
    }

    void undo()
    {
        mPath->texture() = mOldParams;
        mChanger->afterSetPathTextureParams(mPath, mNewParams);
    }

    bool merge(ProjectChange *other)
    {
        SetPathTextureParams *o = (SetPathTextureParams*) other;
        if (!(o->mPath == mPath && o->mDiff == mDiff))
            return false;
        mNewParams = o->mNewParams;
        return true;
    }

    int id() const { return ID_SetPathTextureParams; }

    QString text() const
    {
        if (mDiff & DiffTexture) return mChanger->tr("Change Path Texture");
        if (mDiff & DiffScaleX) return mChanger->tr("Change Path Texture X Scale");
        if (mDiff & DiffScaleY) return mChanger->tr("Change Path Texture Y Scale");
        if (mDiff & DiffTranslateX) return mChanger->tr("Change Path Texture X Shift");
        if (mDiff & DiffTranslateY) return mChanger->tr("Change Path Texture Y Shift");
        if (mDiff & DiffRotate) return mChanger->tr("Change Path Texture Rotation");
        if (mDiff & DiffAlign) return mChanger->tr("Change Path Texture Alignment");
        return mChanger->tr("Change Texture Parameters");
    }

    Path *mPath;
    PathTexture mNewParams;
    PathTexture mOldParams;
    int mDiff;
};
#endif
class SetPathStroke : public ProjectChange
{
public:
    SetPathStroke(ProjectChanger *changer, Path *path, qreal stroke) :
        ProjectChange(changer),
        mPath(path),
        mNewStroke(stroke),
        mOldStroke(path->strokeWidth())
    {
    }

    void redo()
    {
        mPath->setStrokeWidth(mNewStroke);
        mChanger->afterSetPathStroke(mPath, mOldStroke);
    }

    void undo()
    {
        mPath->setStrokeWidth(mOldStroke);
        mChanger->afterSetPathStroke(mPath, mNewStroke);
    }

    int id() const { return ID_SetPathStroke; }

    bool merge(ProjectChange *other)
    {
        SetPathStroke *o = (SetPathStroke*) other;
        if (!(o->mPath == mPath))
            return false;
        mNewStroke = o->mNewStroke;
        return true;
    }

    QString text() const
    {
        return mChanger->tr("Set Path Stroke Width");
    }

    Path *mPath;
    qreal mNewStroke;
    qreal mOldStroke;
};

class SetPathColor : public ProjectChange
{
public:
    SetPathColor(ProjectChanger *changer, Path *path, const QColor &color) :
        ProjectChange(changer),
        mPath(path),
        mNewColor(color),
        mOldColor(path->color())
    {
    }

    void redo()
    {
        mPath->setColor(mNewColor);
        mChanger->afterSetPathColor(mPath, mOldColor);
    }

    void undo()
    {
        mPath->setColor(mOldColor);
        mChanger->afterSetPathColor(mPath, mNewColor);
    }

    int id() const { return ID_SetPathColor; }

    bool merge(ProjectChange *other)
    {
        SetPathColor *o = (SetPathColor*) other;
        if (!(o->mPath == mPath))
            return false;
        mNewColor = o->mNewColor;
        return true;
    }

    QString text() const
    {
        return mChanger->tr("Set Path Color");
    }

    Path *mPath;
    QColor mNewColor;
    QColor mOldColor;
};

class SetLayerVisible : public ProjectChange
{
public:
    SetLayerVisible(ProjectChanger *changer, Layer *layer, bool visible) :
        ProjectChange(changer),
        mLayer(layer),
        mVisible(visible),
        mWasVisible(layer->isVisible())
    {
    }

    void redo()
    {
        mLayer->setVisible(mVisible);
        mChanger->afterSetLayerVisible(mLayer, mVisible);
    }

    void undo()
    {
        mLayer->setVisible(mWasVisible);
        mChanger->afterSetLayerVisible(mLayer, mWasVisible);
    }

    QString text() const
    {
        return mChanger->tr(mVisible ? "Show Layer" : "Hide Layer");
    }

    Layer *mLayer;
    bool mVisible;
    bool mWasVisible;
};

class AddImage : public ProjectChange
{
public:
    AddImage(ProjectChanger *changer, ImagesLayer *layer, int index, ProjectImage *pimg) :
        ProjectChange(changer),
        mLayer(layer),
        mIndex(index),
        mImage(pimg)
    {
    }

    void redo()
    {
        mLayer->insertImage(mIndex, mImage);
        mChanger->afterAddImage(mLayer, mIndex, mImage);
    }

    void undo()
    {
        mChanger->beforeRemoveImage(mLayer, mIndex, mImage);
        mLayer->removeImage(mIndex);
        mChanger->afterRemoveImage(mLayer, mIndex, mImage);
    }

    QString text() const
    {
        return mChanger->tr("Add Image");
    }

    ImagesLayer *mLayer;
    int mIndex;
    ProjectImage *mImage;
};

class RemoveImage : public AddImage
{
public:
    RemoveImage(ProjectChanger *changer, ImagesLayer *layer, int index) :
        AddImage(changer, layer, index, layer->imageAt(index))
    { }
    void redo() { AddImage::undo(); }
    void undo() { AddImage::redo(); }
    QString text() const { return mChanger->tr("Remove Image"); }
};

class MoveImage : public ProjectChange
{
public:
    MoveImage(ProjectChanger *changer, ProjectImage *pimg, const QPointF &pos) :
        ProjectChange(changer),
        mImage(pimg),
        mNewPos(pos),
        mOldPos(pimg->pos())
    {

    }

    void redo()
    {
        mImage->setPos(mNewPos);
        mChanger->afterMoveImage(mImage, mOldPos);
    }

    void undo()
    {
        mImage->setPos(mOldPos);
        mChanger->afterMoveImage(mImage, mNewPos);
    }

    QString text() const
    {
        return mChanger->tr("Move Image");
    }

    ProjectImage *mImage;
    QPointF mNewPos;
    QPointF mOldPos;
};

class ChangeImage : public ProjectChange
{
public:
    ChangeImage(ProjectChanger *changer, ProjectImage *pimg,
                ProjectChanger::ImageChange change,
                const QVariant &newValue, const QVariant &oldValue) :
        ProjectChange(changer),
        mImage(pimg),
        mChange(change),
        mNewValue(newValue),
        mOldValue(oldValue)
    {

    }

    void redo()
    {
        swap(mNewValue, mOldValue);
    }

    void undo()
    {
        swap(mOldValue, mNewValue);
    }

    void swap(const QVariant &newValue, const QVariant &oldValue)
    {
        switch (mChange) {
        case ProjectChanger::ImageChangeFile:
            mImage->setFileName(newValue.toString());
            mImage->setImage(QImage(mImage->fileName()));
            break;
        case ProjectChanger::ImageChangeVisible: mImage->setVisible(newValue.toBool()); break;
        case ProjectChanger::ImageChangeScale: mImage->setScale(newValue.toDouble()); break;
        case ProjectChanger::ImageChangeOpacity: mImage->setOpacity(newValue.toDouble()); break;
        case ProjectChanger::ImageChangeBmpIndex: mImage->setBmpIndex(newValue.toInt()); break;
        }
        mChanger->afterChangeImage(mImage, mChange, oldValue);
    }

    QString text() const
    {
        switch (mChange) {
        case ProjectChanger::ImageChangeFile: return mChanger->tr("Change Image File");
        case ProjectChanger::ImageChangeVisible: return mChanger->tr("Change Image Visibility");
        case ProjectChanger::ImageChangeScale: return mChanger->tr("Change Image Scale");
        case ProjectChanger::ImageChangeOpacity: return mChanger->tr("Change Image Opacity");
        case ProjectChanger::ImageChangeBmpIndex: return mChanger->tr("Change Image BMP");
        }
        return QString();
    }

    ProjectImage *mImage;
    ProjectChanger::ImageChange mChange;
    QVariant mNewValue;
    QVariant mOldValue;
};

class Paint : public ProjectChange
{
public:
    Paint(ProjectChanger *changer, PaintLayer *layer, const QPoint &pos, const QImage &image) :
        ProjectChange(changer),
        mLayer(layer),
        mPos(pos),
        mSource(pos, image),
        mRegion(QRect(pos, image.size()))
    {
        mErased = ResizableImage(mPos, layer->image().copy(pos.x(), pos.y(), image.width(), image.height()));
    }

    void redo()
    {
        QPainter p(&mLayer->image());
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setClipRegion(mRegion);
        p.drawImage(mRegion.boundingRect().topLeft(), mSource, QRect(QPoint(), mSource.mSize));
        mChanger->afterPaint(mLayer, mRegion.boundingRect());
    }

    void undo()
    {
        QPainter p(&mLayer->image());
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setClipRegion(mRegion);
        p.drawImage(mRegion.boundingRect().topLeft(), mErased, QRect(QPoint(), mErased.mSize));
        mChanger->afterPaint(mLayer, mRegion.boundingRect());
    }

    int id() const { return ID_Paint; }

    bool merge(ProjectChange *other)
    {
        Paint *o = (Paint*) other;
        if (!(o->mLayer == mLayer))
            return false;

        const QRegion newRegion = o->mRegion.subtracted(mRegion);
        const QRegion combinedRegion = mRegion.united(o->mRegion);
        const QRect bounds = QRect(mPos.x(), mPos.y(), mSource.mSize.width(), mSource.mSize.height());
        const QRect combinedBounds = combinedRegion.boundingRect();

        // Resize the erased tiles and source image when necessary
        if (bounds != combinedBounds) {
            const QPoint shift = bounds.topLeft() - combinedBounds.topLeft();
            mErased.resize(combinedBounds.size(), shift);
            mSource.resize(combinedBounds.size(), shift);
        }

        mPos = combinedBounds.topLeft();
        mRegion = combinedRegion;

        // Copy the painted pixels from the other command over
        mSource.merge(&o->mSource, o->mRegion);

        // Copy the newly-erased pixels from the other command over
        mErased.merge(&o->mErased, newRegion);

        // Simplify the region to a single rectangle by copying areas
        // outside the painted/erased region
#if 0
        qDebug() << "mRegion #rects=" << mRegion.rectCount();
#else
        QRegion outside = QRegion(mRegion.boundingRect()) - mRegion;
        QPainter pS(&mSource), pE(&mErased);
        pS.setCompositionMode(QPainter::CompositionMode_Source);
        pE.setCompositionMode(QPainter::CompositionMode_Source);
        foreach (QRect r, outside.rects()) {
            pS.drawImage(r.translated(-mPos), mLayer->image(), r);
            pE.drawImage(r.translated(-mPos), mLayer->image(), r);
        }
        mRegion |= outside;
        qDebug() << "mRegion #rects=" << mRegion.rectCount() << "outside #rects=" << outside.rectCount();
#endif

        return true;
    }

    QString text() const
    {
        return mChanger->tr("Paint");
    }

    PaintLayer *mLayer;
    QPoint mPos;
    ResizableImage mSource;
    ResizableImage mErased;
    QRegion mRegion;
};
#endif

} // namespace ProjectChanges;

/////

ProjectChange::ProjectChange(ProjectChanger *changer) :
    mChanger(changer)
{
}

ProjectChange::~ProjectChange()
{
}

void ProjectChange::setChanger(ProjectChanger *changer)
{
    mChanger = changer;
}

using namespace ProjectChanges;

/////

ProjectChangeUndoCommand::ProjectChangeUndoCommand(ProjectChange *change, bool mergeable) :
    QUndoCommand(change->text()),
    mChange(change),
    mMergeable(mergeable)
{
}

ProjectChangeUndoCommand::~ProjectChangeUndoCommand()
{
    delete mChange;
}

void ProjectChangeUndoCommand::redo()
{
    mChange->redo();
}

bool ProjectChangeUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() == id()) {
        ProjectChangeUndoCommand *o = (ProjectChangeUndoCommand*) other;
        if (mChange->id() != -1 && mChange->id() == o->mChange->id() && o->mMergeable)
            return mChange->merge(o->mChange);
    }
    return false;
}

void ProjectChangeUndoCommand::undo()
{
    mChange->undo();
}

int ProjectChangeUndoCommand::id() const
{
    enum { UndoCmd_WorldChange };
    return UndoCmd_WorldChange;
}

/////

ProjectChanger::ProjectChanger(Project *prj) :
    mProject(prj),
    mUndoStack(0),
#ifndef QT_NO_DEBUG
    mUndoMacroDepth(0),
    mUndoCommandDepth(0),
#endif
    mUndoMergeable(false)
{
}

ProjectChanger::~ProjectChanger()
{
    qDeleteAll(mChanges);
}

#if 0
void ProjectChanger::doMoveNode(Node *node, const QPointF &pos)
{
    addChange(new MoveNode(this, node, pos));
}

void ProjectChanger::afterMoveNode(Node *node, const QPointF &prev)
{
    emit afterMoveNodeSignal(node, prev);
}

void ProjectChanger::doMoveNodes(const NodeList &nodes, const QList<QPointF> &posList)
{
    Q_ASSERT(nodes.size() == posList.size());
    addChange(new MoveNodes(this, nodes, posList));
}

void ProjectChanger::afterMoveNodes(const NodeList &nodes, const QList<QPointF> &prev)
{
    Q_ASSERT(nodes.size() == prev.size());
    emit afterMoveNodesSignal(nodes, prev);
}

void ProjectChanger::doAddPath(PathLayer *layer, int index, Path *path)
{
    addChange(new AddPath(this, layer, index, path));
}

void ProjectChanger::doRemovePath(PathLayer *layer, int index, Path *path)
{
    addChange(new RemovePath(this, layer, index, path));
}

void ProjectChanger::afterAddPath(PathLayer *layer, int index, Path *path)
{
    emit afterAddPathSignal(layer, index, path);
}

void ProjectChanger::beforeRemovePath(PathLayer *layer, int index, Path *path)
{
    emit beforeRemovePathSignal(layer, index, path);
}

void ProjectChanger::afterRemovePath(PathLayer *layer, int index, Path *path)
{
    emit afterRemovePathSignal(layer, index, path);
}

void ProjectChanger::doAddNodeToPath(Path *path, int index, Node *node)
{
    addChange(new AddNodeToPath(this, path, index, node));
}

void ProjectChanger::doRemoveNodeFromPath(Path *path, int index, Node *node)
{
    addChange(new RemoveNodeFromPath(this, path, index, node));
}

void ProjectChanger::afterAddNodeToPath(Path *path, int index, Node *node)
{
    emit afterAddNodeToPathSignal(path, index, node);
}

void ProjectChanger::beforeRemoveNodeFromPath(Path *path, int index, Node *node)
{
    emit beforeRemoveNodeFromPathSignal(path, index, node);
}

void ProjectChanger::afterRemoveNodeFromPath(Path *path, int index, Node *node)
{
    emit afterRemoveNodeFromPathSignal(path, index, node);
}

void ProjectChanger::doSetPathClosed(Path *path, bool closed)
{
    addChange(new SetPathClosed(this, path, closed));
}

void ProjectChanger::afterSetPathClosed(Path *path, bool wasClosed)
{
    emit afterSetPathClosedSignal(path, wasClosed);
}

void ProjectChanger::doAddScriptToPath(Path *path, int index, WorldScript *script)
{
    addChange(new AddScriptToPath(this, path, index, script));
}

void ProjectChanger::afterAddScriptToPath(Path *path, int index, WorldScript *script)
{
    emit afterAddScriptToPathSignal(path, index, script);
}

void ProjectChanger::afterRemoveScriptFromPath(Path *path, int index, WorldScript *script)
{
    emit afterRemoveScriptFromPathSignal(path, index, script);
}
#if 0
void ProjectChanger::doSetPathTextureParams(Path *path, const PathTexture &params)
{
    addChange(new SetPathTextureParams(this, path, params));
}
void ProjectChanger::afterSetPathTextureParams(Path *path, const PathTexture &params)
{
    emit afterSetPathTextureParamsSignal(path, params);
}
#endif
void ProjectChanger::doSetPathStroke(Path *path, qreal stroke)
{
    addChange(new SetPathStroke(this, path, stroke));
}

void ProjectChanger::afterSetPathStroke(Path *path, qreal oldStroke)
{
    emit afterSetPathStrokeSignal(path, oldStroke);
}

void ProjectChanger::doSetPathColor(Path *path, const QColor &color)
{
    addChange(new SetPathColor(this, path, color));
}

void ProjectChanger::afterSetPathColor(Path *path, const QColor &color)
{
    emit afterSetPathColorSignal(path, color);
}

void ProjectChanger::doAddLayer(int index, Layer *layer)
{
    addChange(new AddLayer(this, index, layer));
}

void ProjectChanger::doRemoveLayer(int index, Layer *layer)
{
    addChange(new RemoveLayer(this, index, layer));
}

void ProjectChanger::afterAddLayer(int index, Layer *layer)
{
    emit afterAddLayerSignal(index, layer);
}

void ProjectChanger::beforeRemoveLayer(int index, Layer *layer)
{
    emit beforeRemoveLayerSignal(index, layer);
}

void ProjectChanger::afterRemoveLayer(int index, Layer *layer)
{
    emit afterRemoveLayerSignal(index, layer);
}

void ProjectChanger::doReorderLayer(Layer *layer, int newIndex)
{
    addChange(new ReorderLayer(this, layer, newIndex));
}

void ProjectChanger::afterReorderLayer(Layer *layer, int oldIndex)
{
    emit afterReorderLayerSignal(layer, oldIndex);
}

void ProjectChanger::doChangeScriptParameters(WorldScript *script, const ScriptParams &params)
{
    addChange(new ChangeScriptParameters(this, script, params));
}

void ProjectChanger::afterChangeScriptParameters(WorldScript *script)
{
    emit afterChangeScriptParametersSignal(script);
}

void ProjectChanger::doSetPathVisible(Path *path, bool visible)
{
    addChange(new SetPathVisible(this, path, visible));
}

void ProjectChanger::afterSetPathVisible(Path *path, bool visible)
{
    emit afterSetPathVisibleSignal(path, visible);
}

void ProjectChanger::doReorderPath(Path *path, int newIndex)
{
    addChange(new ReorderPath(this, path, newIndex));
}

void ProjectChanger::afterReorderPath(Path *path, int oldIndex)
{
    emit afterReorderPathSignal(path, oldIndex);
}

void ProjectChanger::doSetLayerVisible(Layer *layer, bool visible)
{
    addChange(new SetLayerVisible(this, layer, visible));
}

void ProjectChanger::afterSetLayerVisible(Layer *layer, bool visible)
{
    emit afterSetLayerVisibleSignal(layer, visible);
}

void ProjectChanger::doRenameLayer(Layer *layer, const QString &name)
{
    addChange(new RenameLayer(this, layer, name));
}

void ProjectChanger::afterRenameLayer(Layer *layer, const QString &oldName)
{
    emit afterRenameLayerSignal(layer, oldName);
}

void ProjectChanger::doAddImage(ImagesLayer *layer, int index, ProjectImage *pimg)
{
    addChange(new AddImage(this, layer, index, pimg));
}

void ProjectChanger::doRemoveImage(ImagesLayer *layer, int index)
{
    addChange(new RemoveImage(this, layer, index));
}

void ProjectChanger::afterAddImage(ImagesLayer *layer, int index, ProjectImage *pimg)
{
    emit afterAddImageSignal(layer, index, pimg);
}

void ProjectChanger::beforeRemoveImage(ImagesLayer *layer, int index, ProjectImage *pimg)
{
    emit beforeRemoveImageSignal(layer, index, pimg);
}

void ProjectChanger::afterRemoveImage(ImagesLayer *layer, int index, ProjectImage *pimg)
{
    emit afterRemoveImageSignal(layer, index, pimg);
}

void ProjectChanger::doMoveImage(ProjectImage *pimg, const QPointF &newPos)
{
    addChange(new MoveImage(this, pimg, newPos));
}

void ProjectChanger::afterMoveImage(ProjectImage *pimg, const QPointF &oldPos)
{
    emit afterMoveImageSignal(pimg, oldPos);
}

void ProjectChanger::doSetImageFile(ProjectImage *pimg, const QString &fileName)
{
    addChange(new ChangeImage(this, pimg, ImageChangeFile, fileName, pimg->fileName()));
}

void ProjectChanger::doSetImageVisible(ProjectImage *pimg, bool visible)
{
    addChange(new ChangeImage(this, pimg, ImageChangeVisible, visible, pimg->isVisible()));
}

void ProjectChanger::doSetImageScale(ProjectImage *pimg, qreal scale)
{
    addChange(new ChangeImage(this, pimg, ImageChangeScale, scale, pimg->scale()));
}

void ProjectChanger::doSetImageOpacity(ProjectImage *pimg, qreal opacity)
{
    addChange(new ChangeImage(this, pimg, ImageChangeOpacity, opacity, pimg->opacity()));
}

void ProjectChanger::doSetImageBmpIndex(ProjectImage *pimg, int bmpIndex)
{
    addChange(new ChangeImage(this, pimg, ImageChangeBmpIndex, bmpIndex, pimg->bmpIndex()));
}

void ProjectChanger::afterChangeImage(ProjectImage *pimg, ProjectChanger::ImageChange change,
                                      const QVariant &oldValue)
{
    emit afterChangeImageSignal(pimg, change, oldValue);
}

void ProjectChanger::doPaint(PaintLayer *layer, const QPoint &pos, const QImage &image)
{
    addChange(new Paint(this, layer, pos, image));
}

void ProjectChanger::afterPaint(PaintLayer *layer, const QRect &rect)
{
    emit afterPaintSignal(layer, rect);
}
#endif

ProjectChangeList ProjectChanger::takeChanges(bool undoFirst)
{
    if (undoFirst) undo();
    ProjectChangeList changes = mChanges;
    mChanges.clear();
    mChangesReversed.clear();
    return changes;
}

void ProjectChanger::undoAndForget()
{
    qDeleteAll(takeChanges());
}

void ProjectChanger::beginUndoMacro(QUndoStack *undoStack, const QString &text)
{
    Q_ASSERT(mUndoStack == 0);
#ifndef QT_NO_DEBUG
    Q_ASSERT(mUndoMacroDepth == 0);
    mUndoMacroDepth++;
#endif
    mUndoStack = undoStack;
    mUndoStack->beginMacro(text);
}

void ProjectChanger::endUndoMacro()
{
    Q_ASSERT(mUndoStack != 0);
#ifndef QT_NO_DEBUG
    Q_ASSERT(mUndoMacroDepth == 1);
    mUndoMacroDepth--;
#endif
    mUndoStack->endMacro();
    mUndoStack = 0;
}

void ProjectChanger::beginUndoCommand(QUndoStack *undoStack, bool mergeable)
{
    Q_ASSERT(mUndoStack == 0);
#ifndef QT_NO_DEBUG
    Q_ASSERT(mUndoCommandDepth == 0);
    mUndoCommandDepth++;
#endif
    mUndoStack = undoStack;
    mUndoMergeable = mergeable;
}

void ProjectChanger::endUndoCommand()
{
    Q_ASSERT(mUndoStack != 0);
#ifndef QT_NO_DEBUG
    Q_ASSERT(mUndoCommandDepth == 1);
    mUndoCommandDepth--;
#endif
    mUndoStack = 0;
}

void ProjectChanger::undo()
{
    foreach (ProjectChange *c, mChangesReversed)
        c->undo();
}

void ProjectChanger::doAddNode(int index, BaseNode *node)
{
    addChange(new AddNode(this, index, node));
}

void ProjectChanger::doRemoveNode(BaseNode *node)
{
    int index = mProject->rootNode()->indexOf(node);
    Q_ASSERT(index != -1);
    addChange(new RemoveNode(this, index, node));
}

void ProjectChanger::doMoveNode(BaseNode *node, const QPointF &pos)
{
    addChange(new MoveNode(this, node, pos));
}

void ProjectChanger::doAddConnection(int index, NodeConnection *cxn)
{
    addChange(new AddConnection(this, index, cxn));
}

void ProjectChanger::doRemoveConnection(BaseNode *node, NodeConnection *cxn)
{
    int index = node->indexOf(cxn);
    Q_ASSERT(index != -1);
    addChange(new RemoveConnection(this, index, cxn));
}

void ProjectChanger::doAddVariable(int index, ScriptVariable *var)
{
    addChange(new AddVariable(this, index, var));
}

void ProjectChanger::doRemoveVariable(ScriptVariable *var)
{
    int index = mProject->rootNode()->variables().indexOf(var);
    Q_ASSERT(index != -1);
    addChange(new RemoveVariable(this, index, var));
}

void ProjectChanger::doChangeVariable(ScriptVariable *var, const ScriptVariable *newVar)
{
    addChange(new ChangeVariable(this, var, newVar));
}

void ProjectChanger::doSetVariableValue(ScriptVariable *var, const QString &value)
{
    ScriptVariable newVar(var);
    newVar.setValue(value);
    addChange(new ChangeVariable(this, var, newVar));
}

void ProjectChanger::doSetVariableRef(ScriptVariable *var, const QString &varName)
{
    ScriptVariable newVar(var);
    newVar.setVariableRef(varName);
    addChange(new ChangeVariable(this, var, newVar));
}

void ProjectChanger::addChange(ProjectChange *change)
{
    if (mUndoStack != 0) {
        mUndoStack->push(new ProjectChangeUndoCommand(change, mUndoMergeable));
        return;
    }
    mChanges += change;
    mChangesReversed.insert(0, change);
    change->redo();
}
