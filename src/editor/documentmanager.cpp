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

#include "documentmanager.h"

#include "document.h"

#include <QDebug>
#include <QFileInfo>
//#include <QUndoGroup>

SINGLETON_IMPL(DocumentManager)

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
//    , mUndoGroup(new QUndoGroup(this))
    , mCurrent(0)
    , mFailedToAdd(false)
{
}

DocumentManager::~DocumentManager()
{
}

void DocumentManager::addDocument(Document *doc)
{
    int insertAt = mDocuments.size();
    mDocuments.insert(insertAt, doc);

//    mUndoGroup->addStack(doc->undoStack());
    mFailedToAdd = false;
    emit documentAdded(doc);
    if (mFailedToAdd) {
        closeDocument(insertAt);
        return;
    }
    setCurrentDocument(doc);
}

void DocumentManager::closeDocument(int index)
{
    Q_ASSERT(index >= 0 && index < mDocuments.size());

    Document *doc = mDocuments.takeAt(index);
    emit documentAboutToClose(index, doc);
    if (doc == mCurrent) {
        if (mDocuments.size())
            setCurrentDocument(mDocuments.first());
        else
            setCurrentDocument((Document*)0);
    }
    delete doc;
}

void DocumentManager::closeDocument(Document *doc)
{
    closeDocument(mDocuments.indexOf(doc));
}

void DocumentManager::closeCurrentDocument()
{
    if (mCurrent)
        closeDocument(indexOf(mCurrent));
}

void DocumentManager::closeAllDocuments()
{
    while (documentCount())
        closeCurrentDocument();
}

int DocumentManager::findDocument(const QString &fileName)
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return -1;

    for (int i = 0; i < mDocuments.size(); ++i) {
        QFileInfo fileInfo(mDocuments.at(i)->fileName());
        if (fileInfo.canonicalFilePath() == canonicalFilePath)
            return i;
    }

    return -1;
}

void DocumentManager::setCurrentDocument(int index)
{
    Q_ASSERT(index >= -1 && index < mDocuments.size());
    setCurrentDocument((index >= 0) ? mDocuments.at(index) : 0);
}

void DocumentManager::setCurrentDocument(Document *doc)
{
    Q_ASSERT(!doc || mDocuments.contains(doc));

    if (doc == mCurrent)
        return;

    qDebug() << "current document was " << mCurrent << " is becoming " << doc;

    if (mCurrent) {
    }

    mCurrent = doc;

    if (mCurrent) {
//        mUndoGroup->setActiveStack(mCurrent->undoStack());
    }

    emit currentDocumentChanged(doc);
}

Document *DocumentManager::documentAt(int index) const
{
    Q_ASSERT(index >= 0 && index < mDocuments.size());
    return mDocuments.at(index);
}

QList<LuaDocument *> DocumentManager::luaDocuments() const
{
    QList<LuaDocument *> ret;
    foreach (Document *doc, mDocuments)
        if (LuaDocument *ldoc = doc->asLuaDocument())
            ret += ldoc;
    return ret;
}

QList<ProjectDocument *> DocumentManager::projectDocuments() const
{
    QList<ProjectDocument *> ret;
    foreach (Document *doc, mDocuments)
        if (ProjectDocument *ldoc = doc->asProjectDocument())
            ret += ldoc;
    return ret;
}

void DocumentManager::setFailedToAdd()
{
    mFailedToAdd = true;
}

bool DocumentManager::failedToAdd()
{
    return mFailedToAdd;
}
