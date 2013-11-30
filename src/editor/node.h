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

#include <QPointF>
#include <QList>
#include <QMap>
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
        mName(name)
    {
    }

    NodeInput(const NodeInput *other) :
        mNode(0),
        mName(other->mName)
    {
    }

    const QString &name() const { return mName; }

    bool isKnown() const;

    BaseNode *mNode;
    QString mName;
};

class NodeOutput
{
public:
    NodeOutput(const QString &name) :
        mNode(0),
        mName(name)
    {
    }
    NodeOutput(const NodeOutput *other) :
        mNode(0),
        mName(other->mName)
    {
    }

    const QString &name() const { return mName; }

    bool isKnown() const;

    BaseNode *mNode;
    QString mName;
};

class NodeConnection
{
public:
    BaseNode *mSender;
    QString mOutput;
    BaseNode *mReceiver;
    QString mInput;
    // Could have a time delay here
};

// Base of LuaNode and ScriptNode
class BaseNode
{
public:
    BaseNode(int id, const QString &name) :
        mID(id),
        mName(name)
    {
    }

    void setID(int id) { mID = id; }
    int id()const { return mID; }

    void setName(const QString &name) { mName = name; }
    const QString &name() const { return mName; }

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
            if (input->mName == name)
                return input;
        return 0;
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
            if (output->mName == name)
                return output;
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
    void insertConnection(int index, NodeConnection *cxn);
    NodeConnection *removeConnection(NodeConnection *cxn);

    const QList<ScriptVariable*> &variables() const
    {
        return mVariables;
    }
    ScriptVariable *variable(const QString &name);
    int variableCount() const
    {
        return mVariables.size();
    }
    void insertVariable(int index, ScriptVariable *var);
    ScriptVariable *removeVariable(ScriptVariable *var);

    virtual bool isKnown(const ScriptVariable *var) = 0;
    virtual bool isKnown(const NodeInput *input) = 0;
    virtual bool isKnown(const NodeOutput *output) = 0;

    void initFrom(BaseNode *other);

    virtual bool isLuaNode() { return false; }
    virtual LuaNode *asLuaNode() { return NULL; }

    virtual bool isScriptNode() { return false; }
    virtual ScriptNode *asScriptNode() { return NULL; }

protected:
    int mID;
    QString mName;
    QList<ScriptVariable*> mVariables;
    QList<NodeInput*> mInputs;
    QList<NodeOutput*> mOutputs;
    QList<NodeConnection*> mConnections;
    QPointF mPosition;
//    QString mComment;
};

class LuaNode : public BaseNode
{
public:
    LuaNode(int id, const QString &name);

    bool isKnown(const ScriptVariable *var);
    bool isKnown(const NodeInput *input);
    bool isKnown(const NodeOutput *output);

    void setSource(const QString &path) { mSource = path; }
    QString source() { return mSource; }

    void initFrom(LuaNode *other);

    virtual bool isLuaNode() { return true; }
    virtual LuaNode *asLuaNode() { return this; }

    QString mSource; // (relative) path to .lua
    LuaInfo *mDefinition;
};

class ScriptNode : public BaseNode
{
public:
    ScriptNode(int id, const QString &name) :
        BaseNode(id, name),
        mInfo(0)
    {
    }

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
    void insertNode(int index, BaseNode *object)
    {
        mNodes.insert(index, object);
    }
    BaseNode *removeNode(int index)
    {
        return mNodes.takeAt(index);
    }
    BaseNode *nodeByID(int id)
    {
        foreach (BaseNode *node, mNodes)
            if (node->id() == id)
                return node;
        return NULL;
    }

    void setScriptInfo(ScriptInfo *info) { mInfo = info; }
    ScriptInfo *scriptInfo() const { return mInfo; }

    void setSource(const QString &path) { mSource = path; }
    QString source() { return mSource; }

    bool syncWithScriptInfo();

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
