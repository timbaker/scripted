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

#ifndef EDITOR_GLOBAL_H
#define EDITOR_GLOBAL_H

#include <QDebug>
#include <QList>

class AbstractTool;
class BaseGraphicsScene;
class BaseGraphicsView;
class BaseNode;
class Document;
class NodeConnection;
class LuaNode;
class LuaInfo;
class NodeInput;
class NodeInputItem;
class NodeInputGroupItem;
class NodeOutput;
class NodeOutputItem;
class NodeOutputGroupItem;
class ScriptNode;
class NodeItem;
class EmbeddedMainWindow;
class ScriptScene;
class ScriptView;
class Project;
class ProjectChange;
typedef QList<ProjectChange*> ProjectChangeList;
class ProjectChanger;
class ProjectDocument;
class ProjectReader;
class ScriptInfo;
class ScriptVariable;

typedef ScriptScene ProjectScene;
typedef ScriptView ProjectView;

#ifdef QT_NO_DEBUG
inline QNoDebug noise() { return QNoDebug(); }
#else
inline QDebug noise() { return QDebug(QtDebugMsg); }
#endif

#endif // EDITOR_GLOBAL_H
