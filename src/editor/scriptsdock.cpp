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

#include "scriptsdock.h"
#include "ui_scriptsdock.h"

#include "projectactions.h"
#include "preferences.h"

#include <QFileSystemModel>
#include <QHeaderView>

ScriptsDock::ScriptsDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScriptsDock)
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
    filters << QLatin1String("*.pzs");
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
#else
    hHeader->setResizeMode(0, QHeaderView::Stretch);
#endif

    connect(ui->treeView, SIGNAL(activated(QModelIndex)), SLOT(activated(QModelIndex)));
    connect(ui->dirComboBox, SIGNAL(currentIndexChanged(int)), SLOT(dirSelected(int)));
    connect(prefs(), SIGNAL(gameDirectoriesChanged()), SLOT(gameDirectoriesChanged()));

    setDirCombo();
}

void ScriptsDock::gameDirectoriesChanged()
{
    setDirCombo();
}

void ScriptsDock::activated(const QModelIndex &index)
{
    if (mModel->isDir(index)) return;
    QString fileName = mModel->filePath(index);
    ProjectActions::instance()->openProject(fileName);
}

void ScriptsDock::dirSelected(int index)
{
    if (index != -1) {
        QDir dir(prefs()->gameDirectories().at(index));
        dir = dir.filePath("media/metascripts");
        if (dir.exists()) {
            mModel->setRootPath(dir.absolutePath());
            ui->treeView->setRootIndex(mModel->index(mModel->rootPath()));
            return;
        }
    }
    mModel->setRootPath(qApp->applicationDirPath());
    ui->treeView->setRootIndex(mModel->index(mModel->rootPath()));
}

void ScriptsDock::setDirCombo()
{
    ui->dirComboBox->clear();
    foreach (QString path, prefs()->gameDirectories()) {
        QFileInfo info(path);
        ui->dirComboBox->addItem(info.fileName());
    }
}
