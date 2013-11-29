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

#include <QBoxLayout>
#include <QCompleter>
#include <QDirModel>
#include <QEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QToolButton>

ScriptsView::ScriptsView(QWidget *parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setDragEnabled(true);
    setDefaultDropAction(Qt::MoveAction);

    connect(prefs(), SIGNAL(scriptsDirectoryChanged()), this, SLOT(onScriptsDirectoryChanged()));

    QDir mapsDir(prefs()->scriptsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());

    QFileSystemModel *model = mFSModel = new QFileSystemModel;
    model->setRootPath(mapsDir.absolutePath());

    model->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
    QStringList filters;
    filters << QLatin1String("*.pzs");
    model->setNameFilters(filters);
    model->setNameFilterDisables(false); // hide filtered files

    setModel(model);

    QHeaderView* hHeader = header();
    hHeader->hideSection(1); // Size
    hHeader->hideSection(2);
    hHeader->hideSection(3);

    setRootIndex(model->index(mapsDir.absolutePath()));

    header()->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
    header()->setResizeMode(0, QHeaderView::Stretch);
    header()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(onActivated(QModelIndex)));
}

QSize ScriptsView::sizeHint() const
{
    return QSize(130, 100);
}

void ScriptsView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        // Prevent drag-and-drop starting when clicking on an unselected item.
        setDragEnabled(selectionModel()->isSelected(index));

        // Hack: disable dragging folders.
        // FIXME: the correct way to do this would be to override the flags()
        // method of QFileSystemModel.
        if (model()->isDir(index))
            setDragEnabled(false);
    }

    QTreeView::mousePressEvent(event);
}

void ScriptsView::onScriptsDirectoryChanged()
{
    QDir mapsDir(prefs()->scriptsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());
    model()->setRootPath(mapsDir.canonicalPath());
    setRootIndex(model()->index(mapsDir.absolutePath()));
}

void ScriptsView::onActivated(const QModelIndex &index)
{
    QString path = model()->filePath(index);
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        prefs()->setScriptsDirectory(fileInfo.canonicalFilePath());
    }
    if (fileInfo.suffix() == QLatin1String("pzs"))
        ProjectActions::instance()->openProject(fileInfo.canonicalFilePath());
}

/////

ScriptsDock::ScriptsDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScriptsDock)
{
    ui->setupUi(this);
}
