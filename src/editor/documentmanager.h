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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "editor_global.h"
#include "singleton.h"
#include <QObject>

class QUndoGroup;

class DocumentManager : public QObject, public Singleton<DocumentManager>
{
    Q_OBJECT

public:
    explicit DocumentManager(QObject *parent = 0);
    ~DocumentManager();

    void addDocument(Document *doc);
    void closeDocument(int index);
    void closeDocument(Document *doc);

    void closeCurrentDocument();
    void closeAllDocuments();

    int findDocument(const QString &fileName);

    void setCurrentDocument(int index);
    void setCurrentDocument(Document *doc);
    Document *currentDocument() const { return mCurrent; }

    const QList<Document*> &documents() const { return mDocuments; }
    int documentCount() const { return mDocuments.size(); }

    Document *documentAt(int index) const;
    int indexOf(Document *doc) { return mDocuments.indexOf(doc); }

    QList<LuaDocument*> luaDocuments() const;
    QList<ProjectDocument*> projectDocuments() const;

//    QUndoGroup *undoGroup() const { return mUndoGroup; }

    void setFailedToAdd();
    bool failedToAdd();

signals:
    void currentDocumentChanged(Document *doc);
    void documentAdded(Document *doc);
    void documentAboutToClose(int index, Document *doc);

private:
    QList<Document*> mDocuments;
//    QUndoGroup *mUndoGroup;
    Document *mCurrent;
    bool mFailedToAdd;
};

inline DocumentManager *docman() { return DocumentManager::instance(); }

#endif // DOCUMENTMANAGER_H
