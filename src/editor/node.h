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

#ifndef NODE_H
#define NODE_H

#include "editor_global.h"
#include "scriptvariable.h"

#include <QList>
#include <QMap>
#include <QPolygonF>
#include <QStringList>

class BaseNode;
class LuaNode;
class ScriptNode;
class ScriptVariable;

class NodeInput
{
public:
    NodeInput(const QString &name) :
        mNode(0),
        mName(name),
        mLabel(name)
    {
    }

    NodeInput(const QString &name, const QString &label) :
        mNode(0),
        mName(name),
        mLabel(label)
    {
    }

    NodeInput(const NodeInput *other) :
        mNode(0),
        mName(other->mName),
        mLabel(other->mLabel)
    {
    }

    NodeInput(const NodeInput *other, BaseNode *node) :
        mNode(node),
        mName(other->mName),
        mLabel(other->mLabel)
    {
    }

    void setName(const QString &name) { mName = name; }
    const QString &name() const { return mName; }

    void setLabel(const QString &label) { mLabel = label; }
    const QString &label() const { return mLabel; }

    bool isKnown() const;

    void setNode(BaseNode *node) { mNode = node; }
    BaseNode *node() const { return mNode; }

    void initFrom(const NodeInput *other)
    {
        mNode = other->mNode;
        mName = other->mName;
        mLabel = other->mLabel;
    }

private:
    BaseNode *mNode;
    QString mName;
    QString mLabel;
};

class NodeOutput
{
public:
    NodeOutput(const QString &name) :
        mNode(0),
        mName(name),
        mLabel(name)
    {
    }

    NodeOutput(const QString &name, const QString &label) :
        mNode(0),
        mName(name),
        mLabel(label)
    {
    }

    NodeOutput(const NodeOutput *other) :
        mNode(0),
        mName(other->mName),
        mLabel(other->mLabel)
    {
    }

    NodeOutput(const NodeOutput *other, BaseNode *node) :
        mNode(node),
        mName(other->mName),
        mLabel(other->mLabel)
    {
    }

    void setName(const QString &name) { mName = name; }
    const QString &name() const { return mName; }

    void setLabel(const QString &label) { mLabel = label; }
    const QString &label() const { return mLabel; }

    bool isKnown() const;

    void setNode(BaseNode *node) { mNode = node; }
    BaseNode *node() const { return mNode; }

    void initFrom(const NodeOutput *other)
    {
        mNode = other->mNode;
        mName = other->mName;
        mLabel = other->mLabel;
    }

private:
    BaseNode *mNode;
    QString mName;
    QString mLabel;
};

class NodeConnection
{
public:
    BaseNode *mSender;
    QString mOutput;
    BaseNode *mReceiver;
    QString mInput;
    QPolygonF mControlPoints;
    // Could have a time delay here
};

// Base of LuaNode and ScriptNode
class BaseNode
{
public:
    BaseNode(int id, const QString &name) :
        mID(id),
        mIsProjectRootNode(false),
        mLabel(name)
    {
    }

    virtual ~BaseNode();

    void setID(int id) { mID = id; }
    int id()const { return mID; }

    void setLabel(const QString &label) { mLabel = label; }
    const QString &label() const { return mLabel; }

    void setPos(qreal x, qreal y) { mPosition = QPointF(x, y); }
    void setPos(const QPointF &pos) { mPosition = pos; }
    QPointF pos() const { return mPosition; }

    void insertInput(int index, NodeInput *input);
    NodeInput *removeInput(int index);
    int inputCount() { return mInputs.size(); }
    const QList<NodeInput*> &inputs() const { return mInputs; }
    NodeInput *input(int index)
    {
        if (index >= 0 && index < mInputs.size())
            return mInputs[index];
        return 0;
    }
    NodeInput *input(const QString &name)
    {
        foreach (NodeInput *input, mInputs)
            if (input->name() == name)
                return input;
        return 0;
    }
    int indexOf(NodeInput *input)
    {
        return mInputs.indexOf(input);
    }

    void insertOutput(int index, NodeOutput *output);
    NodeOutput *removeOutput(int index);
    int outputCount() { return mOutputs.size(); }
    const QList<NodeOutput*> &outputs() const { return mOutputs; }
    NodeOutput *output(int index)
    {
        if (index >= 0 && index < mOutputs.size())
            return mOutputs[index];
        return 0;
    }
    NodeOutput *output(const QString &name)
    {
        foreach (NodeOutput *output, mOutputs)
            if (output->name() == name)
                return output;
        return 0;
    }
    int indexOf(NodeOutput *output)
    {
        return mOutputs.indexOf(output);
    }

    NodeConnection *connection(int index) const
    {
        if (index >= 0 && index < mConnections.size())
            return mConnections[index];
        return 0;
    }
    const QList<NodeConnection*> &connections() const
    {
        return mConnections;
    }
    int connectionCount() const
    {
        return mConnections.size();
    }
    int indexOf(NodeConnection *cxn)
    {
        return mConnections.indexOf(cxn);
    }
    void insertConnection(int index, NodeConnection *cxn);
    NodeConnection *removeConnection(NodeConnection *cxn);

    const QList<ScriptVariable*> &variables() const
    {
        return mVariables;
    }
    ScriptVariable *variable(const QString &label);
    int variableCount() const
    {
        return mVariables.size();
    }
    int indexOf(ScriptVariable *var)
    {
        return mVariables.indexOf(var);
    }
    void insertVariable(int index, ScriptVariable *var);
    ScriptVariable *removeVariable(ScriptVariable *var);

    virtual bool isKnown(const ScriptVariable *var) = 0;
    virtual bool isKnown(const NodeInput *input) = 0;
    virtual bool isKnown(const NodeOutput *output) = 0;

    void initFrom(const BaseNode *other);

    virtual bool isEventNode() { return false; }
    virtual MetaEventNode *asEventNode() { return NULL; }

    virtual bool isLuaNode() { return false; }
    virtual LuaNode *asLuaNode() { return NULL; }

    virtual bool isScriptNode() { return false; }
    virtual ScriptNode *asScriptNode() { return NULL; }

    bool syncWithInfo(BaseNode *infoNode);

    void setProjectRootNode() {  mIsProjectRootNode = true; }
    bool isProjectRootNode() { return mIsProjectRootNode; }

protected:
    int mID;
    bool mIsProjectRootNode;
    QString mLabel;
    QList<ScriptVariable*> mVariables;
    QList<NodeInput*> mInputs;
    QList<NodeOutput*> mOutputs;
    QList<NodeConnection*> mConnections;
    QPointF mPosition;
//    QString mComment;
};

class MetaEventNode : public BaseNode
{
public:
    MetaEventNode(int id, const QString &label);
    MetaEventNode(int id, const MetaEventNode &other);

    bool isKnown(const ScriptVariable *var);
    bool isKnown(const NodeInput *input);
    bool isKnown(const NodeOutput *output);

    void setInfo(MetaEventInfo *info) { mInfo = info; }
    MetaEventInfo *info() const { return mInfo; }

    bool syncWithInfo();

    void initFrom(const MetaEventNode *other);

    virtual bool isEventNode() { return true; }
    virtual MetaEventNode *asEventNode() { return this; }

private:
    MetaEventInfo *mInfo;
};

class LuaNode : public BaseNode
{
public:
    LuaNode(int id, const QString &label);
    LuaNode(int id, const LuaNode &other);

    bool isKnown(const ScriptVariable *var);
    bool isKnown(const NodeInput *input);
    bool isKnown(const NodeOutput *output);

    void setSource(const QString &path) { mSource = path; }
    QString source() { return mSource; }

    using BaseNode::syncWithInfo;
    bool syncWithInfo();

    void initFrom(const LuaNode *other);

    virtual bool isLuaNode() { return true; }
    virtual LuaNode *asLuaNode() { return this; }

    QString mSource; // (relative) path to .lua
    LuaInfo *mInfo;
};

class ScriptNode : public BaseNode
{
public:
    ScriptNode(int id, const QString &name) :
        BaseNode(id, name),
        mInfo(0)
    {
    }

    ~ScriptNode();

    int nodeCount() const
    {
        return mNodes.size();
    }
    const QList<BaseNode*> &nodes()
    {
        return mNodes;
    }
    BaseNode *node(int index)
    {
        return (index >= 0 && index < mNodes.size()) ? mNodes.at(index) : 0;
    }
    void insertNode(int index, BaseNode *node)
    {
        Q_ASSERT(nodeByID(node->id()) == 0);
        mNodes.insert(index, node);
    }
    BaseNode *removeNode(int index)
    {
        return mNodes.takeAt(index);
    }
    BaseNode *nodeByID(int id)
    {
        if (id == mID) return this;
        foreach (BaseNode *node, mNodes)
            if (node->id() == id)
                return node;
        return NULL;
    }
    int indexOf(BaseNode *node)
    {
        return mNodes.indexOf(node);
    }
    using BaseNode::indexOf;

    QList<BaseNode*> nodesPlusSelf()
    {
        QList<BaseNode*> ret = mNodes;
        ret.prepend(this);
        return ret;
    }

    void setInfo(ScriptInfo *info) { mInfo = info; }
    ScriptInfo *info() const { return mInfo; }

    void setSource(const QString &path) { mSource = path; }
    QString source() { return mSource; }

    using BaseNode::syncWithInfo;
    bool syncWithInfo();

    bool isKnown(const ScriptVariable *var);
    bool isKnown(const NodeInput *input);
    bool isKnown(const NodeOutput *output);

    void initFrom(ScriptNode *other);

    virtual bool isScriptNode() { return true; }
    virtual ScriptNode *asScriptNode() { return this; }

private:
    QString mSource; // (relative) path to .pzs
    ScriptInfo *mInfo;
    QList<BaseNode*> mNodes;
};

#endif // NODE_H
