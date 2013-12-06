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

#include "luadocument.h"

#include "documentmanager.h"
#include "luaeditor.h"
#include "mainwindow.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUndoStack>

LuaDocument::LuaDocument(const QString &fileName) :
    Document(LuaDocType),
    mFileName(fileName),
    mEditor(0)
{
     // This isn't used, due to LuaEditor having its own inaccessible QUndoStack
    mUndoStack = new QUndoStack(this);
    //    connect(mUndoStack, SIGNAL(cleanChanged(bool)), SIGNAL(cleanChanged()));
}

bool LuaDocument::isModified() const
{
    return mEditor ? mEditor->document()->isModified() : false;
}

void LuaDocument::setFileName(const QString &fileName)
{
    if (fileName == mFileName)
        return;
    mFileName = fileName;
    emit fileNameChanged();
}

const QString &LuaDocument::fileName() const
{
    return mFileName;
}

bool LuaDocument::save(const QString &filePath, QString &error)
{
    if (mEditor) {
        QTemporaryFile tempFile;
        if (!tempFile.open(/*QIODevice::WriteOnly | QIODevice::Text*/)) {
            error = tempFile.errorString();
            return false;
        }
        QTextStream ts(&tempFile);
        ts << mEditor->toPlainText();

        if (tempFile.error() != QFile::NoError) {
            error = tempFile.errorString();
            return false;
        }

        // foo.lua -> foo.lua.bak
        QFileInfo destInfo(filePath);
        QString backupPath = filePath + QLatin1String(".bak");
        QFile backupFile(backupPath);
        if (destInfo.exists()) {
            if (backupFile.exists()) {
                if (!backupFile.remove()) {
                    error = QString(QLatin1String("Error deleting file!\n%1\n\n%2"))
                            .arg(backupPath)
                            .arg(backupFile.errorString());
                    return false;
                }
            }
            QFile destFile(filePath);
            if (!destFile.rename(backupPath)) {
                error = QString(QLatin1String("Error renaming file!\nFrom: %1\nTo: %2\n\n%3"))
                        .arg(filePath)
                        .arg(backupPath)
                        .arg(destFile.errorString());
                return false;
            }
        }

        // /tmp/tempXYZ -> foo.lua
        tempFile.close();
        if (!tempFile.rename(filePath)) {
            error = QString(QLatin1String("Error renaming file!\nFrom: %1\nTo: %2\n\n%3"))
                    .arg(tempFile.fileName())
                    .arg(filePath)
                    .arg(tempFile.errorString());
            // Try to un-rename the backup file
            if (backupFile.exists())
                backupFile.rename(filePath); // might fail
            return false;
        }

        bool keepBackup = false;
        if (!keepBackup && backupFile.exists())
            backupFile.remove();

        // If anything above failed, the temp file should auto-remove, but not after
        // a successful save.
        tempFile.setAutoRemove(false);
    }

    error.clear();
    undoStack()->setClean(); // useless
    setFileName(filePath);
    return true;
}

void LuaDocument::setEditor(LuaEditor *editor)
{
    if (mEditor)
        mEditor->disconnect(this);

    mEditor = editor;

    if (mEditor) {
        if (!fileName().isEmpty()) {
            QFile file(fileName());
            if (file.open(QFile::ReadOnly | QFile::Text))
                mEditor->setPlainText(file.readAll());
            else {
                QMessageBox::critical(mainwin(), tr("Error Reading Lua"),
                                      tr("%1\n%2")
                                      .arg(QDir::toNativeSeparators(fileName()))
                                      .arg(file.errorString()));
                docman()->setFailedToAdd();
            }
        }
        connect(mEditor, SIGNAL(modificationChanged(bool)), SIGNAL(cleanChanged()));
    }
}
