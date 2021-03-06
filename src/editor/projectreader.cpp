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

#include "projectreader.h"

#include "luamanager.h"
#include "node.h"
#include "project.h"
#include "scriptvariable.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

class ProjectReaderPrivate
{
    Q_DECLARE_TR_FUNCTIONS(ProjectReaderPrivate)

public:
    ProjectReaderPrivate()
        : mProject(0)
    {

    }

    bool openFile(QFile *file)
    {
        if (!file->exists()) {
            mError = tr("File not found: %1").arg(file->fileName());
            return false;
        } else if (!file->open(QFile::ReadOnly | QFile::Text)) {
            mError = tr("Unable to read file: %1").arg(file->fileName());
            return false;
        }

        return true;
    }

    QString errorString() const
    {
        if (!mError.isEmpty()) {
            return mError;
        } else {
            return tr("%3\n\nLine %1, column %2")
                    .arg(xml.lineNumber())
                    .arg(xml.columnNumber())
                    .arg(xml.errorString());
        }
    }

    Project *readProject(QIODevice *device, const QString &path)
    {
        mError.clear();
        mRelativeTo = QFileInfo(path).absolutePath();
        Project *project = 0;

        xml.setDevice(device);

        if (xml.readNextStartElement() && xml.name() == QLatin1String("script")) {
            project = readProject();
            if (project)
                project->rootNode()->setLabel(QFileInfo(path).baseName());
        } else {
            xml.raiseError(tr("Not a script file."));
        }

        return project;
    }

private:
    Project *readProject()
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("script"));

        const QXmlStreamAttributes atts = xml.attributes();
        const int version = atts.value(QLatin1String("version")).toString().toInt();

        mProject = new Project();
        int nextID = mProject->mNextID;

        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("connection")) {
                if (NodeConnection *cxn = readConnection(mProject->mRootNode))
                    mProject->rootNode()->insertConnection(mProject->rootNode()->connectionCount(), cxn);
            } else if (xml.name() == QLatin1String("input")) {
                if (NodeInput *input = readInput(mProject->mRootNode))
                    mProject->rootNode()->insertInput(mProject->rootNode()->inputCount(), input);
            } else if (xml.name() == QLatin1String("output")) {
                if (NodeOutput *output = readOutput(mProject->mRootNode))
                    mProject->rootNode()->insertOutput(mProject->rootNode()->outputCount(), output);
            } else if (xml.name() == QLatin1String("event-node")) {
                if (MetaEventNode *node = readEventNode()) {
                    mProject->rootNode()->insertNode(mProject->rootNode()->nodeCount(), node);
                    if (node->id() >= nextID)
                        nextID = node->id() + 1;
                }
            } else if (xml.name() == QLatin1String("lua-node")) {
                if (LuaNode *node = readLuaNode()) {
                    mProject->rootNode()->insertNode(mProject->rootNode()->nodeCount(), node);
                    if (node->id() >= nextID)
                        nextID = node->id() + 1;
                }
            } else if (xml.name() == QLatin1String("script-node")) {
                if (ScriptNode *node = readScriptNode()) {
                    mProject->rootNode()->insertNode(mProject->rootNode()->nodeCount(), node);
                    if (node->id() >= nextID)
                        nextID = node->id() + 1;
                }
            } else if (xml.name() == QLatin1String("variable")) {
                if (ScriptVariable *v = readVariable(true)) {
                    mProject->rootNode()->insertVariable(mProject->rootNode()->variableCount(), v);
                }
            } else
                readUnknownElement();

            mProject->mNextID = nextID;
        }

        if (xml.hasError()) {
            delete mProject;
            return 0;
        }

        foreach (BaseNode *node, mProject->rootNode()->nodesPlusSelf()) {
            foreach (ScriptVariable *var, node->variables()) {
                if (var->variableRef().length()) {
                    BaseNode *node = mProject->rootNode()->nodeByID(var->variableRefID());
                    if (!node) {
                        mError = tr("Invalid referenceid \"%1\" (unknown node)").arg(var->variableRefID());
                        break;
                    }
                }
            }
            if (!mError.isEmpty())
                break;
            foreach (NodeConnection *cxn, node->connections()) {
                int id = (int)cxn->mReceiver;
                if (BaseNode *rcvr = mProject->rootNode()->nodeByID(id)) {
#if 0 // yep it is allowed
                    if (rcvr == node) {
                        mError = tr("Node \"%1\" can't form a connection with itself").arg(id);
                        break;
                    }
#endif
                    cxn->mReceiver = rcvr;
                } else {
                    mError = tr("Invalid receiver \"%1\"").arg(id); // FIXME: line number
                    break;
                }
            }
            if (!mError.isEmpty())
                break;
        }

        // Clean up in case of error
        if (!mError.isEmpty() || xml.hasError()) {
            delete mProject;
            mProject = 0;
        }

        return mProject;
    }

    MetaEventNode *readEventNode()
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("event-node"));

        const QXmlStreamAttributes atts = xml.attributes();
        bool ok;
        int id = atts.value(QLatin1String("id")).toInt(&ok);
        if (!ok || id < 0) {
            xml.raiseError(tr("missing or invalid node ID"));
            return 0;
        }
        const QString eventName = atts.value(QLatin1String("eventname")).toString();
        if (eventName.isEmpty()) {
            xml.raiseError(tr("Empty or missing event name"));
            return 0;
        }
        QString label = atts.value(QLatin1String("label")).toString();
        if (label.isEmpty())
            label = eventName;
        double x, y;
        if (!getDoublePair(atts, QLatin1String("pos"), x, y))
            return 0;

        MetaEventNode *node = new MetaEventNode(id, eventName, label);
        node->setPos(x, y);

        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("connection")) {
                if (NodeConnection *cxn = readConnection(node))
                    node->insertConnection(node->connectionCount(), cxn);
#if 0
            } else if (xml.name() == QLatin1String("input")) {
                if (NodeInput *input = readInput())
                    node->insertInput(node->inputCount(), input);
            } else if (xml.name() == QLatin1String("output")) {
                if (NodeOutput *output = readOutput())
                    node->insertOutput(node->outputCount(), output);
#endif
            } else if (xml.name() == QLatin1String("source")) {
                const QXmlStreamAttributes atts = xml.attributes();
                node->setSource(getRelativeFile(atts, "file"));
                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("variable")) {
                if (ScriptVariable *v = readVariable(false)) {
                    node->insertVariable(node->variableCount(), v);
                }
            } else
                readUnknownElement();
        }

        return node;
    }

    LuaNode *readLuaNode()
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("lua-node"));

        const QXmlStreamAttributes atts = xml.attributes();
        bool ok;
        int id = atts.value(QLatin1String("id")).toInt(&ok);
        if (!ok || id < 0) {
            xml.raiseError(tr("missing or invalid node ID"));
            return 0;
        }
        const QString label = atts.value(QLatin1String("label")).toString();
        double x, y;
        if (!getDoublePair(atts, QLatin1String("pos"), x, y))
            return 0;

        LuaNode *node = new LuaNode(id, label);
        node->setPos(x, y);

        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("connection")) {
                if (NodeConnection *cxn = readConnection(node))
                    node->insertConnection(node->connectionCount(), cxn);
            } else if (xml.name() == QLatin1String("input")) {
                if (NodeInput *input = readInput(node))
                    node->insertInput(node->inputCount(), input);
            } else if (xml.name() == QLatin1String("output")) {
                if (NodeOutput *output = readOutput(node))
                    node->insertOutput(node->outputCount(), output);
            } else if (xml.name() == QLatin1String("source")) {
                const QXmlStreamAttributes atts = xml.attributes();
                node->setSource(getRelativeFile(atts, "file"));
                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("variable")) {
                if (ScriptVariable *v = readVariable(false)) {
                    node->insertVariable(node->variableCount(), v);
                }
            } else
                readUnknownElement();
        }

        return node;
    }

    ScriptNode *readScriptNode()
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("script-node"));

        const QXmlStreamAttributes atts = xml.attributes();
        bool ok;
        int id = atts.value(QLatin1String("id")).toInt(&ok);
        if (!ok || id < 0) {
            xml.raiseError(tr("missing or invalid node ID"));
            return 0;
        }
        const QString label = atts.value(QLatin1String("label")).toString();
        double x, y;
        if (!getDoublePair(atts, QLatin1String("pos"), x, y))
            return 0;

        ScriptNode *node = new ScriptNode(id, label);
        node->setPos(x, y);

        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("connection")) {
                if (NodeConnection *cxn = readConnection(node))
                    node->insertConnection(node->connectionCount(), cxn);
            } else if (xml.name() == QLatin1String("input")) {
                if (NodeInput *input = readInput(node))
                    node->insertInput(node->inputCount(), input);
            } else if (xml.name() == QLatin1String("output")) {
                if (NodeOutput *output = readOutput(node))
                    node->insertOutput(node->outputCount(), output);
            } else if (xml.name() == QLatin1String("source")) {
                const QXmlStreamAttributes atts = xml.attributes();
                node->setSource(getRelativeFile(atts, "file"));
                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("variable")) {
                if (ScriptVariable *p = readVariable(false)) {
                    node->insertVariable(node->variableCount(), p);
                }
            } else
                readUnknownElement();
        }

        return node;
    }

    NodeInput *readInput(BaseNode *node)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("input"));
        const QXmlStreamAttributes atts = xml.attributes();
        QString name = atts.value(QLatin1String("name")).toString();
        if (name.isEmpty()) {
            xml.raiseError(tr("Empty or missing input name"));
            return NULL;
        }
        QString label = name;
        if (node->isProjectRootNode()) {
            label = atts.value(QLatin1String("label")).toString();
            if (label.isEmpty())
                label = name;
        }
        xml.skipCurrentElement();
        return new NodeInput(name, label);
    }

    NodeOutput *readOutput(BaseNode *node)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("output"));
        const QXmlStreamAttributes atts = xml.attributes();
        QString name = atts.value(QLatin1String("name")).toString();
        if (name.isEmpty()) {
            xml.raiseError(tr("Empty or missing output name"));
            return NULL;
        }
        QString label = name;
        if (node->isProjectRootNode()) {
            label = atts.value(QLatin1String("label")).toString();
            if (label.isEmpty())
                label = name;
        }
        xml.skipCurrentElement();
        return new NodeOutput(name);
    }

    NodeConnection *readConnection(BaseNode *node)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("connection"));
        const QXmlStreamAttributes atts = xml.attributes();
        QString outputName = atts.value(QLatin1String("output")).toString();
        bool ok;
        QString s = atts.value(QLatin1String("receiver")).toString();
        int rcvr = s.toInt(&ok);
        if (!ok || rcvr < 0) {
            xml.raiseError(tr("Expected receiver=int but got \"%2\"").arg(s));
            return 0;
        }
        QString inputName = atts.value(QLatin1String("input")).toString();
        QPolygonF points;
        if (!getPolygonF(atts, QLatin1String("controlpoints"), points))
            return 0;

        NodeConnection *cxn = new NodeConnection;
        cxn->mSender = node;
        cxn->mOutput = outputName;
        cxn->mInput = inputName;
        cxn->mReceiver = (ScriptNode*)rcvr;
        cxn->mControlPoints = points;
        xml.skipCurrentElement();
        return cxn;
    }

    ScriptVariable *readVariable(bool rootNode)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("variable"));
        const QXmlStreamAttributes atts = xml.attributes();
        QString type = atts.value(QLatin1String("type")).toString();
        QString name = atts.value(QLatin1String("name")).toString();
        QString label = rootNode ? atts.value(QLatin1String("label")).toString() : name;
        if (atts.hasAttribute(QLatin1String("value"))) {
            QString value = atts.value(QLatin1String("value")).toString();
            xml.skipCurrentElement();
            return new ScriptVariable(type, name, label, value);
        } else {
            QString idStr = atts.value(QLatin1String("referenceid")).toString();
            bool ok;
            int varRefID = idStr.toInt(&ok);
            if (!ok) {
                xml.raiseError(tr("Expected integer node-id but got \"%1\"").arg(idStr));
                return 0;
            }
            QString varRef = atts.value(QLatin1String("referencevar")).toString();
            xml.skipCurrentElement();
            return new ScriptVariable(type, name, name, varRefID, varRef);
        }
    }

    void readUnknownElement()
    {
        qDebug() << "Unknown element (fixme):" << xml.name();
        xml.skipCurrentElement();
    }

    QString resolveReference(const QString &fileName, const QString &relativeTo)
    {
//        qDebug() << "resolveReference" << fileName << "relative to" << relativeTo;
        if (fileName.isEmpty())
            return fileName;
        if (fileName == QLatin1String("."))
            return relativeTo;
        if (QDir::isRelativePath(fileName)) {
            QString path = QDir(relativeTo).filePath(fileName);
            QFileInfo info(path);
            if (info.exists())
                return info.canonicalFilePath();
            return QDir::cleanPath(path);
        }
        return fileName;
    }

    QString getRelativeFile(const QXmlStreamAttributes &atts, const QString &name)
    {
        QString s = atts.value(name).toString();
        return resolveReference(s, mRelativeTo);
    }

    bool getBoolean(const QXmlStreamAttributes &atts, const QString &name, bool defaultValue)
    {
        if (atts.hasAttribute(name)) {
            QString s = atts.value(name).toString();
            if (s == QLatin1String("true")) return true;
            if (s == QLatin1String("false")) return false;
            xml.raiseError(tr("Expected boolean but got \"%1\"").arg(s));
        }
        return defaultValue;
    }

    bool getDouble(const QXmlStreamAttributes &atts, const QString &name, double &v)
    {
        QString s = atts.value(name).toString();
        if (atts.hasAttribute(name)) {
            bool ok;
            v = s.toDouble(&ok);
            if (ok) return v;
        }
        xml.raiseError(tr("Expected %1=double but got \"%2\"").arg(name).arg(s));
        return false;
    }

    bool getDoublePair(const QString &s, double &v1, double &v2)
    {
        QStringList vs = s.split(QLatin1Char(','), QString::SkipEmptyParts);
        if (vs.size() == 2) {
            bool ok;
            v1 = vs[0].toDouble(&ok);
            if (!ok) return false;
            v2 = vs[1].toDouble(&ok);
            if (!ok) return false;
            return true;
        }
        return false;
    }

    bool getDoublePair(const QXmlStreamAttributes &atts, const QString &name,
                       double &v1, double &v2)
    {
        if (atts.hasAttribute(name))
            if (getDoublePair(atts.value(name).toString(), v1, v2))
                return true;
bogus:
        xml.raiseError(tr("Expected %1=double,double but got \"%2\"")
                       .arg(name).arg(atts.value(name).toString()));
        return false;
    }

    bool getPolygonF(const QXmlStreamAttributes &atts, const QString &name,
                     QPolygonF &points)
    {
        if (!atts.hasAttribute(name))
            return true;

        points.clear();
        QStringList vs = atts.value(name).toString().split(QLatin1Char(' '), QString::SkipEmptyParts);
        foreach (QString v, vs) {
            double x, y;
            if (!getDoublePair(v, x, y))
                goto bogus;
            points += QPointF(x, y);
        }

        return true;
bogus:
        xml.raiseError(tr("Expected %1=double,double but got \"%2\"")
                       .arg(name).arg(atts.value(name).toString()));
        return false;
    }

private:
    QString mRelativeTo;
    Project *mProject;
    QString mError;
    QXmlStreamReader xml;
};

/////

ProjectReader::ProjectReader() :
    d(new ProjectReaderPrivate)
{
}

ProjectReader::~ProjectReader()
{
    delete d;
}

Project *ProjectReader::read(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return 0;

    return d->readProject(&file, fileName);
}

QString ProjectReader::errorString() const
{
    return d->errorString();
}
