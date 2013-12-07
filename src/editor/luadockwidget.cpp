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

#include "luadockwidget.h"
#include "ui_luadockwidget.h"

#include "luamanager.h"
#include "node.h"
#include "preferences.h"
#include "projectactions.h"

#include <QFileInfo>
#include <QFileSystemModel>

LuaDockWidget::LuaDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::LuaDockWidget)
{
    ui->setupUi(this);

    QTreeView *t = ui->treeView;

    t->setRootIsDecorated(false);
    t->setHeaderHidden(true);
    t->setItemsExpandable(true);
    t->setUniformRowHeights(true);
    t->setDragEnabled(true);
    t->setDefaultDropAction(Qt::CopyAction);

    mModel = new QFileSystemModel;
    mModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
    QStringList filters;
    filters << QLatin1String("*.lua");
    mModel->setNameFilters(filters);
    mModel->setNameFilterDisables(false); // hide filtered files
    t->setModel(mModel);

    QHeaderView* hHeader = t->header();
    hHeader->hideSection(1); // Size
    hHeader->hideSection(2);
    hHeader->hideSection(3);

    hHeader->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    hHeader->setSectionResizeMode(0, QHeaderView::Stretch);
//    hHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
    hHeader->setResizeMode(0, QHeaderView::Stretch);
//    hHeader->setResizeMode(1, QHeaderView::ResizeToContents);
#endif

    connect(ui->treeView, SIGNAL(activated(QModelIndex)), SLOT(activated(QModelIndex)));
    connect(ui->dirComboBox, SIGNAL(currentIndexChanged(int)), SLOT(dirSelected(int)));
    connect(prefs(), SIGNAL(gameDirectoriesChanged()), SLOT(gameDirectoriesChanged()));

    setDirCombo();
}

LuaDockWidget::~LuaDockWidget()
{
    delete ui;
}

void LuaDockWidget::disableDragAndDrop()
{
    ui->treeView->setDragEnabled(false);
}

void LuaDockWidget::gameDirectoriesChanged()
{
    setDirCombo();
}

void LuaDockWidget::activated(const QModelIndex &index)
{
    QString fileName = mModel->filePath(index);
    ProjectActions::instance()->openLuaFile(fileName);
}

void LuaDockWidget::dirSelected(int index)
{
    if (index != -1) {
        QDir dir(prefs()->gameDirectories().at(index));
        dir = dir.filePath("media/lua/MetaGame");
        if (dir.exists()) {
            mModel->setRootPath(dir.absolutePath());
            ui->treeView->setRootIndex(mModel->index(mModel->rootPath()));
            return;
        }
    }
    mModel->setRootPath(qApp->applicationDirPath());
    ui->treeView->setRootIndex(mModel->index(mModel->rootPath()));
}

void LuaDockWidget::setDirCombo()
{
    ui->dirComboBox->clear();
    foreach (QString path, prefs()->gameDirectories()) {
        QFileInfo info(path);
        ui->dirComboBox->addItem(info.fileName());
    }
}
