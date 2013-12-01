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

#ifndef UNDOREDOBUTTONS_H
#define UNDOREDOBUTTONS_H

#include <QObject>

class QToolButton;
class QUndoStack;

class UndoRedoButtons : public QObject
{
    Q_OBJECT
public:
    UndoRedoButtons(QUndoStack *undoStack, QObject *parent = 0);

    void resetIndex();

    QToolButton *undoButton() const
    { return mUndo; }

    QToolButton *redoButton() const
    { return mRedo; }

public slots:
    void updateActions();
    void textChanged();
    void retranslateUi();

private:
    QUndoStack *mUndoStack;
    QToolButton *mUndo;
    QToolButton *mRedo;
    int mUndoIndex;
};

#endif // UNDOREDOBUTTONS_H
