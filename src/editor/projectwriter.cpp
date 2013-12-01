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

#include "projectwriter.h"

#include "luamanager.h"
#include "node.h"
#include "project.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QXmlStreamWriter>

class ProjectWriterPrivate
{
    Q_DECLARE_TR_FUNCTIONS(ProjectWriterPrivate)

public:
    ProjectWriterPrivate()
        : mProject(0)
    {
    }

    bool openFile(QFile *file)
    {
        if (!file->open(QIODevice::WriteOnly)) {
            mError = tr("Could not open file for writing.");
            return false;
        }

        return true;
    }

    void writeProject(Project *project, QIODevice *device, const QString &absDirPath)
    {
        mMapDir = QDir(absDirPath);
        mProject = project;

        xml.setDevice(device);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(1);

        xml.writeStartDocument();

        writeProject(project);

        xml.writeEndDocument();
    }

    void writeProject(Project *project)
    {
        xml.writeStartElement(QLatin1String("script"));

        xml.writeAttribute(QLatin1String("version"), QLatin1String("1"));

        ScriptNode *node = project->rootNode();
        foreach (ScriptVariable *var, node->variables())
            writeVariable(var);
        foreach (NodeInput *input, node->inputs())
            writeInput(input);
        foreach (NodeOutput *output, node->outputs())
            writeOutput(output);
        foreach (BaseNode *child, node->nodes())
            writeNode(child);

        xml.writeEndElement(); // script
    }

    void writeNode(BaseNode *node)
    {
        if (LuaNode *lnode = node->asLuaNode())
            writeLuaNode(lnode);
        else if (ScriptNode *snode = node->asScriptNode())
            writeScriptNode(snode);
    }

    void writeLuaNode(LuaNode *node)
    {
        xml.writeStartElement(QLatin1String("lua-node"));
        xml.writeAttribute(QLatin1String("id"), QString::number(node->id()));
        xml.writeAttribute(QLatin1String("name"), node->name());
        writeDoublePair(QLatin1String("pos"), node->pos().x(), node->pos().y());

        xml.writeStartElement(QLatin1String("source"));
        xml.writeAttribute(QLatin1String("file"), relativeFileName(node->mDefinition
                           ? node->mDefinition->path()
                           : node->source()));
        xml.writeEndElement();

        foreach (ScriptVariable *var, node->variables())
            writeVariable(var);
        foreach (NodeInput *input, node->inputs())
            writeInput(input);
        foreach (NodeOutput *output, node->outputs())
            writeOutput(output);
        foreach (NodeConnection *cxn, node->connections())
            writeConnection(cxn);
        xml.writeEndElement();
    }

    void writeScriptNode(ScriptNode *node)
    {
        xml.writeStartElement(QLatin1String("script-node"));
        xml.writeAttribute(QLatin1String("id"), QString::number(node->id()));
        xml.writeAttribute(QLatin1String("name"), node->name());
        writeDoublePair(QLatin1String("pos"), node->pos().x(), node->pos().y());

        xml.writeStartElement(QLatin1String("source"));
        xml.writeAttribute(QLatin1String("file"), relativeFileName(node->scriptInfo()
                           ? node->scriptInfo()->path()
                           : node->source()));
        xml.writeEndElement();

        foreach (ScriptVariable *var, node->variables())
            writeVariable(var);
        foreach (NodeInput *input, node->inputs())
            writeInput(input);
        foreach (NodeOutput *output, node->outputs())
            writeOutput(output);
        foreach (NodeConnection *cxn, node->connections())
            writeConnection(cxn);
        foreach (BaseNode *child, node->nodes()) {
            // don't write these, they are separate documents
        }
        xml.writeEndElement();

    }

    void writeInput(NodeInput *input)
    {
        xml.writeStartElement(QLatin1String("input"));
        xml.writeAttribute(QLatin1String("name"), input->mName);
        xml.writeEndElement();
    }

    void writeOutput(NodeOutput *output)
    {
        xml.writeStartElement(QLatin1String("output"));
        xml.writeAttribute(QLatin1String("name"), output->mName);
        xml.writeEndElement();
    }

    void writeVariable(ScriptVariable *var)
    {
        xml.writeStartElement(QLatin1String("variable"));
        xml.writeAttribute(QLatin1String("type"), var->type());
        xml.writeAttribute(QLatin1String("name"), var->name());
        if (var->variableRef().isEmpty())
            xml.writeAttribute(QLatin1String("value"), var->value());
        else
            xml.writeAttribute(QLatin1String("reference"), var->variableRef());
        xml.writeEndElement();
    }

    void writeConnection(NodeConnection *cxn)
    {
        xml.writeStartElement(QLatin1String("connection"));
        xml.writeAttribute(QLatin1String("output"), cxn->mOutput);
        xml.writeAttribute(QLatin1String("receiver"), QString::number(cxn->mReceiver->id()));
        xml.writeAttribute(QLatin1String("input"), cxn->mInput);
        xml.writeEndElement();
    }

    void writeBoolean(const QString &name, bool value)
    {
        xml.writeAttribute(name, QLatin1String(value ? "true" : "false"));
    }

    void writeDouble(const QString &name, double v)
    {
        xml.writeAttribute(name, QString::number(v, 'g', 3));
    }

    void writeDoublePair(const QString &name, double v1, double v2)
    {
        xml.writeAttribute(name,
                           QString::number(v1, 'f', 3) +
                           QLatin1String(",") +
                           QString::number(v2, 'f', 3));
    }

    QString relativeFileName(const QString &path)
    {
        if (!path.isEmpty()) {
            QFileInfo fi(path);
            if (fi.isAbsolute())
                return mMapDir.relativeFilePath(path);
        }
        return path;
    }

    QXmlStreamWriter xml;
    Project *mProject;
    QString mError;
    QDir mMapDir;
};

/////

ProjectWriter::ProjectWriter() :
    d(new ProjectWriterPrivate)
{
}

ProjectWriter::~ProjectWriter()
{
    delete d;
}

bool ProjectWriter::write(Project *project, const QString &filePath)
{
    QTemporaryFile tempFile;
    if (!d->openFile(&tempFile))
        return false;

    d->writeProject(project, &tempFile, QFileInfo(filePath).absolutePath());

    if (tempFile.error() != QFile::NoError) {
        d->mError = tempFile.errorString();
        return false;
    }

    // foo.pzs -> foo.pzs.bak
    QFileInfo destInfo(filePath);
    QString backupPath = filePath + QLatin1String(".bak");
    QFile backupFile(backupPath);
    if (destInfo.exists()) {
        if (backupFile.exists()) {
            if (!backupFile.remove()) {
                d->mError = QString(QLatin1String("Error deleting file!\n%1\n\n%2"))
                        .arg(backupPath)
                        .arg(backupFile.errorString());
                return false;
            }
        }
        QFile destFile(filePath);
        if (!destFile.rename(backupPath)) {
            d->mError = QString(QLatin1String("Error renaming file!\nFrom: %1\nTo: %2\n\n%3"))
                    .arg(filePath)
                    .arg(backupPath)
                    .arg(destFile.errorString());
            return false;
        }
    }

    // /tmp/tempXYZ -> foo.pzs
    tempFile.close();
    if (!tempFile.rename(filePath)) {
        d->mError = QString(QLatin1String("Error renaming file!\nFrom: %1\nTo: %2\n\n%3"))
                .arg(tempFile.fileName())
                .arg(filePath)
                .arg(tempFile.errorString());
        // Try to un-rename the backup file
        if (backupFile.exists())
            backupFile.rename(filePath); // might fail
        return false;
    }

    // If anything above failed, the temp file should auto-remove, but not after
    // a successful save.
    tempFile.setAutoRemove(false);

    return true;
}

QString ProjectWriter::errorString() const
{
    return d->mError;
}
