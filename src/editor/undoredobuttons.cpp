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

#include "undoredobuttons.h"

#include <QApplication>
#include <QToolButton>
#include <QUndoStack>

UndoRedoButtons::UndoRedoButtons(QUndoStack *undoStack, QObject *parent) :
    QObject(parent),
    mUndoStack(undoStack)
{
    connect(mUndoStack, SIGNAL(indexChanged(int)), SLOT(updateActions()));

    mUndo = new QToolButton();
    mUndo->setObjectName(QString::fromUtf8("undo"));
    QIcon icon2;
    icon2.addFile(QString::fromUtf8(":/images/16x16/edit-undo.png"), QSize(), QIcon::Normal, QIcon::Off);
    mUndo->setIcon(icon2);
    mUndo->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    mRedo = new QToolButton();
    mRedo->setObjectName(QString::fromUtf8("redo"));
    QIcon icon3;
    icon3.addFile(QString::fromUtf8(":/images/16x16/edit-redo.png"), QSize(), QIcon::Normal, QIcon::Off);
    mRedo->setIcon(icon3);
    mRedo->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(mUndo, SIGNAL(clicked()), mUndoStack, SLOT(undo()));
    connect(mRedo, SIGNAL(clicked()), mUndoStack, SLOT(redo()));
    connect(mUndoStack, SIGNAL(undoTextChanged(QString)), SLOT(textChanged()));
    connect(mUndoStack, SIGNAL(redoTextChanged(QString)), SLOT(textChanged()));

    retranslateUi();

    mUndoIndex = mUndoStack->index();
    updateActions();
}

void UndoRedoButtons::retranslateUi()
{
    mUndo->setText(QApplication::translate("UndoRedoButtons", "Undo"));
    mRedo->setText(QApplication::translate("UndoRedoButtons", "Redo"));
}

void UndoRedoButtons::resetIndex()
{
    mUndoIndex = mUndoStack->index();
    updateActions();
}

void UndoRedoButtons::updateActions()
{
    mUndo->setEnabled(mUndoStack->index() > mUndoIndex);
    mRedo->setEnabled(mUndoStack->canRedo());
}

void UndoRedoButtons::textChanged()
{
    mUndo->setToolTip(mUndoStack->undoText());
    mRedo->setToolTip(mUndoStack->redoText());
}
