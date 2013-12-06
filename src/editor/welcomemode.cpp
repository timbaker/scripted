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

#include "welcomemode.h"
#include "ui_welcomemode.h"

#include "preferences.h"
#include "projectactions.h"

#include <QCompleter>
#include <QDebug>
#include <QDirModel>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QHeaderView>
#include <QLabel>
#include <QSettings>

#define SCENE_WIDTH 400

static QRectF sceneRectOfItem(QGraphicsItem *item)
{
    return item->mapToScene(item->boundingRect()).boundingRect();
}

namespace WelcomeModeNS {

LinkItem::LinkItem(int width, const QString &text1, const QString &text2, QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    QGraphicsRectItem *bg = new QGraphicsRectItem(this);
    bg->setBrush(QColor(QLatin1String("#f3f3f3")));
    bg->setPen(Qt::NoPen);
    bg->setVisible(false);

    QGraphicsTextItem *item1 = new QGraphicsTextItem(this);
    item1->setPlainText(text1);
    item1->setDefaultTextColor(Qt::blue);

    mBoundingRect = sceneRectOfItem(item1);

    if (!text2.isEmpty()) {
        QGraphicsTextItem *item2 = new QGraphicsTextItem(this);
        QString s = QFontMetrics(item2->font()).elidedText(text2, Qt::ElideRight, width);
        item2->setPlainText(s);
        item2->setDefaultTextColor(QColor("#6b6b6b"));
        item2->setPos(0, item1->boundingRect().height());

        mBoundingRect |= sceneRectOfItem(item2);
    }

    mBoundingRect.translate(-mBoundingRect.topLeft());
    mBoundingRect.setWidth(width);
    bg->setRect(mBoundingRect);

    //        setFlag(ItemHasNoContents);
    setAcceptHoverEvents(true);
}

QRectF LinkItem::boundingRect() const
{
    return mBoundingRect;
}

void LinkItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{

}

void LinkItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit clicked();
}

void LinkItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    childItems().first()->setVisible(true);
    emit hovered(true);
}

void LinkItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    childItems().first()->setVisible(false);
    emit hovered(false);
}

}

WelcomeMode::WelcomeMode(QObject *parent) :
    IMode(parent),
    ui(new Ui::WelcomeMode)
{
    setDisplayName(tr("Welcome"));
    setIcon(QIcon(QLatin1String(":images/24x24/document-new.png")));

    mWidget = new QWidget;
    mWidget->setObjectName(QLatin1String("WelcomeModeWidget"));
    ui->setupUi(mWidget);

    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->graphicsView->setMouseTracking(true);

    int x = 10;
    int y = 10;
    QGraphicsPixmapItem *pitem = scene->addPixmap(QPixmap::fromImage(QImage(QLatin1String(":/BuildingEditor/icons/icon_pencil.png"))));
//    QGraphicsPixmapItem *pitem = scene->addPixmap(QPixmap::fromImage(QImage(QLatin1String(":/images/tiled-icon-32.png"))));
    pitem->setPos(x, y + 4);
    QGraphicsTextItem *item = scene->addText(tr("ScriptEd"), QFont(QLatin1String("Helvetica"), 16, 1));
    item->setPos(x + 24 + 6, y);
    QRectF r = sceneRectOfItem(item) | sceneRectOfItem(pitem);
    r.translate(0, 12);
    QGraphicsLineItem *line = scene->addLine(r.left(), r.bottom(), SCENE_WIDTH, r.bottom());
    line->setPen(QPen(Qt::gray));

    x = 40;
    y = r.bottom() + 16;
    WelcomeModeNS::LinkItem *link;
    link = new WelcomeModeNS::LinkItem(160 - x, tr("New Project"));
    scene->addItem(link);
    link->setPos(x + 6, y);
    connect(link, SIGNAL(clicked()), ProjectActions::instance(), SLOT(newProject()));

    y += 30;
    link = new WelcomeModeNS::LinkItem(160 - x, tr("Open Project"));
    scene->addItem(link);
    link->setPos(x + 6, y);
    connect(link, SIGNAL(clicked()), ProjectActions::instance(), SLOT(openProject()));

    y = r.bottom() + 16;
    link = new WelcomeModeNS::LinkItem(160 - x, tr("New Lua File"));
    scene->addItem(link);
    link->setPos(x + 160, y);
    connect(link, SIGNAL(clicked()), ProjectActions::instance(), SLOT(newLuaFile()));

    y += 30;
    link = new WelcomeModeNS::LinkItem(160 - x, tr("Open Lua File"));
    scene->addItem(link);
    link->setPos(x + 160, y);
    connect(link, SIGNAL(clicked()), ProjectActions::instance(), SLOT(openLuaFile()));

    y += sceneRectOfItem(link).height() + 14;
//    QGraphicsLineItem *line2 = scene->addLine(x, y, 300, y);
//    line2->setPen(QPen(Qt::gray));

//    y += 36;
    item = scene->addText(tr("Recent Documents"), QFont(QLatin1String("Helvetica"), 16, 1));
    item->setPos(x, y);

    y += item->boundingRect().height() + 12;
    mRecentItemsY = y;
    setRecentFiles();

    scene->setSceneRect(0, 0, SCENE_WIDTH, 600);

    {
        QDirModel *model = new QDirModel(this);
        model->setFilter(QDir::AllDirs | QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
        QCompleter *completer = new QCompleter(model, this);
        ui->dirEdit->setCompleter(completer);
    }

    connect(prefs(), SIGNAL(scriptsDirectoryChanged()),
            SLOT(scriptsDirectoryChanged()));

    QDir mapsDir(prefs()->scriptsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());

    {
        ui->treeView->setRootIsDecorated(false);
        ui->treeView->setHeaderHidden(true);
        ui->treeView->setItemsExpandable(false);
        ui->treeView->setUniformRowHeights(true);
//        ui->treeView->setDragEnabled(true);
//        ui->treeView->setDefaultDropAction(Qt::MoveAction);

        QFileSystemModel *model = mFSModel = new QFileSystemModel(this);
        model->setRootPath(mapsDir.absolutePath());

        model->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
        model->setNameFilters(QStringList() << QLatin1String("*.pzs"));
        model->setNameFilterDisables(false); // hide filtered files

        ui->treeView->setModel(model);

        QHeaderView* hHeader = ui->treeView->header();
        hHeader->hideSection(1); // Size
        hHeader->hideSection(2);
        hHeader->hideSection(3);

        ui->treeView->setRootIndex(model->index(mapsDir.absolutePath()));

        hHeader->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
        hHeader->setSectionResizeMode(0, QHeaderView::Stretch);
        hHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
        hHeader->setResizeMode(0, QHeaderView::Stretch);
        hHeader->setResizeMode(1, QHeaderView::ResizeToContents);
#endif
        connect(ui->treeView, SIGNAL(activated(QModelIndex)),
                SLOT(onActivated(QModelIndex)));
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                SLOT(selectionChanged()));
    }

    ui->dirEdit->setText(QDir::toNativeSeparators(mapsDir.canonicalPath()));
    connect(ui->dirEdit, SIGNAL(returnPressed()), SLOT(editedProjectsDirectory()));

    connect(ui->dirBrowse, SIGNAL(clicked()), SLOT(browse()));

    setWidget(mWidget);

    connect(prefs(), SIGNAL(recentFilesChanged()),
            SLOT(setRecentFiles()));
}

void WelcomeMode::readSettings(QSettings &settings)
{
    Q_UNUSED(settings)
}

void WelcomeMode::writeSettings(QSettings &settings)
{
    Q_UNUSED(settings)
}

void WelcomeMode::onActivated(const QModelIndex &index)
{
    QString path = mFSModel->filePath(index);
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        prefs()->setScriptsDirectory(fileInfo.canonicalFilePath());
        return;
    }
    ProjectActions::instance()->openFile(path);
}

void WelcomeMode::browse()
{
    QString f = QFileDialog::getExistingDirectory(widget()->window(), tr("Choose the Projects Folder"),
        ui->dirEdit->text());
    if (!f.isEmpty()) {
        prefs()->setScriptsDirectory(f);
    }
}

void WelcomeMode::editedProjectsDirectory()
{
    prefs()->setScriptsDirectory(ui->dirEdit->text());
}

void WelcomeMode::scriptsDirectoryChanged()
{
    QDir mapsDir(prefs()->scriptsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());
    mFSModel->setRootPath(mapsDir.canonicalPath());
    ui->treeView->setRootIndex(mFSModel->index(mapsDir.absolutePath()));
    ui->dirEdit->setText(QDir::toNativeSeparators(prefs()->scriptsDirectory()));
}

void WelcomeMode::selectionChanged()
{
    QModelIndexList selectedRows = ui->treeView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        ui->label->setPixmap(QPixmap());
//        mPreviewMapImage = 0;
        return;
    }
    QModelIndex index = selectedRows.first();
    QString path = mFSModel->filePath(index);
    if (QFileInfo(path).isDir())
        return;
#if 0
    MapImage *mapImage = MapImageManager::instance()->getMapImage(path);
    if (mapImage) {
        if (mapImage->isLoaded()) {
            QImage image = mapImage->image().scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation);
            ui->label->setPixmap(QPixmap::fromImage(image));
        }
    } else
        ui->label->setPixmap(QPixmap());
    mPreviewMapImage = mapImage;
#endif
}

void WelcomeMode::setRecentFiles()
{
    qDeleteAll(mRecentItems);
    mRecentItems.clear();

    int x = 40;
    int y = mRecentItemsY;
    foreach (QString f, prefs()->recentFiles()) {
        WelcomeModeNS::LinkItem *link = new WelcomeModeNS::LinkItem(SCENE_WIDTH - x, QFileInfo(f).fileName(), QDir::toNativeSeparators(f));
        ui->graphicsView->scene()->addItem(link);
        link->setPos(x + 6, y);
        mRecentItems += link;
        connect(link, SIGNAL(clicked()), SLOT(linkClicked()));
        connect(link, SIGNAL(hovered(bool)), SLOT(linkHovered(bool)));
        y += link->boundingRect().height() + 12;
    }

    QRectF sceneRect(0, 0, SCENE_WIDTH, 300);
    if (mRecentItems.size())
        sceneRect |= sceneRectOfItem(mRecentItems.last());
    ui->graphicsView->setSceneRect(sceneRect);
}

void WelcomeMode::linkClicked()
{
    WelcomeModeNS::LinkItem *link = static_cast<WelcomeModeNS::LinkItem *>(sender());
    int index = mRecentItems.indexOf(link);
    if (index >= 0) {
        QString fileName = prefs()->recentFiles().at(index);
        ProjectActions::instance()->openFile(fileName);
    }
}

void WelcomeMode::linkHovered(bool hover)
{
#if 0
    if (hover) {
        WelcomeModeNS::LinkItem *link = static_cast<WelcomeModeNS::LinkItem *>(sender());
        int index = mRecentItems.indexOf(link);
        if (index >= 0) {
            QString fileName = BuildingEditorWindow::instance()->recentFiles().at(index);
            MapImage *mapImage = MapImageManager::instance()->getMapImage(fileName);
            if (mapImage) {
                if (mapImage->isLoaded()) {
                    QImage image = mapImage->image().scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio,
                                                            Qt::SmoothTransformation);
                    ui->label->setPixmap(QPixmap::fromImage(image));
                }
            } else
                ui->label->setPixmap(QPixmap());
            mPreviewMapImage = mapImage;
        }
    } else if (mPreviewMapImage) {
        ui->label->setPixmap(QPixmap());
        mPreviewMapImage = 0;
    }
#endif
}
