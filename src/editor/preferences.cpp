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

#include "preferences.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

SINGLETON_IMPL(Preferences)

static const QLatin1String KEY_SCRIPTS_DIRECTORY("ScriptsDirectory");
static const QLatin1String KEY_GAME_DIRECTORIES("GameDirectories");
static const QLatin1String KEY_USE_OPENGL("UseOpenGL");
static const QLatin1String KEY_SHOW_MINIMAP("ShowMiniMap");
static const QLatin1String KEY_MINIMAP_WIDTH("MiniMapWidth");
static const QLatin1String KEY_BG_COLOR("BackgroundColor");
static const QLatin1String KEY_SHOW_TILE_GRID("ShowTileGrid");
static const QLatin1String KEY_TILE_GRID_COLOR("TileGridColor");
static const QLatin1String KEY_RECENT_FILES("RecentFiles");

Preferences::Preferences() :
    QObject(),
    mSettings(new QSettings)
{
    mScriptsDirectory = mSettings->value(KEY_SCRIPTS_DIRECTORY, QString()).toString();
    mGameDirectories = mSettings->value(KEY_GAME_DIRECTORIES, QStringList()).toStringList();
    mUseOpenGL = mSettings->value(KEY_USE_OPENGL, false).toBool();
    mShowMiniMap = mSettings->value(KEY_SHOW_MINIMAP, true).toBool();
    mMiniMapWidth = mSettings->value(KEY_MINIMAP_WIDTH, 256).toInt();
    mBackgroundColor = QColor(mSettings->value(KEY_BG_COLOR,
                                               QColor(Qt::darkGray).name()).toString());
    mSnapToGrid = true;
    mShowTileGrid = mSettings->value(KEY_SHOW_TILE_GRID, false).toBool();
    mTileGridColor = QColor(mSettings->value(QLatin1String("TileGridColor"),
                                             QColor(Qt::black).name()).toString());

    // Set the default location of the Tiles Directory to the same value set
    // in TileZed's Tilesets Dialog.
    QSettings settings(QLatin1String("mapeditor.org"), QLatin1String("Tiled"));
    QString KEY_TILES_DIR = QLatin1String("Tilesets/TilesDirectory");
    QString tilesDirectory = settings.value(KEY_TILES_DIR).toString();

    if (tilesDirectory.isEmpty() || !QDir(tilesDirectory).exists()) {
        tilesDirectory = QCoreApplication::applicationDirPath() +
                QLatin1Char('/') + QLatin1String("../Tiles");
        if (!QDir(tilesDirectory).exists())
            tilesDirectory = QCoreApplication::applicationDirPath() +
                    QLatin1Char('/') + QLatin1String("../../Tiles");
    }
    if (tilesDirectory.length())
        tilesDirectory = QDir::cleanPath(tilesDirectory);
    if (!QDir(tilesDirectory).exists())
        tilesDirectory.clear();
    mTilesDirectory = mSettings->value(QLatin1String("TilesDirectory"),
                                       tilesDirectory).toString();

    // Use the same directory as TileZed.
    QString KEY_CONFIG_PATH = QLatin1String("ConfigDirectory");
    QString configPath = settings.value(KEY_CONFIG_PATH).toString();
    if (configPath.isEmpty())
        configPath = QDir::homePath() + QLatin1Char('/') + QLatin1String(".TileZed");
    mConfigDirectory = configPath;
}

Preferences::~Preferences()
{
}

QString Preferences::configPath(const QString &fileName) const
{
    return configPath() + QLatin1Char('/') + fileName;
}

QString Preferences::appConfigPath() const
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath();
#elif defined(Q_OS_UNIX)
    return QCoreApplication::applicationDirPath() + QLatin1String("/../share/tilezed/config");
#elif defined(Q_OS_MAC)
    return QCoreApplication::applicationDirPath() + QLatin1String("/../Config");
#else
#error "wtf system is this???"
#endif
}

QString Preferences::appConfigPath(const QString &fileName) const
{
    return appConfigPath() + QLatin1Char('/') + fileName;
}

void Preferences::setScriptsDirectory(const QString &path)
{
    if (path == mScriptsDirectory)
        return;
    mScriptsDirectory = path;
    mSettings->setValue(KEY_SCRIPTS_DIRECTORY, mScriptsDirectory);
    emit scriptsDirectoryChanged();
}

void Preferences::setGameDirectories(const QStringList &dirList)
{
    if (dirList == mGameDirectories)
        return;
    mGameDirectories = dirList;
    mSettings->setValue(KEY_GAME_DIRECTORIES, mGameDirectories);
    emit gameDirectoriesChanged();
}

void Preferences::setUseOpenGL(bool useOpenGL)
{
    if (mUseOpenGL == useOpenGL)
        return;

    mUseOpenGL = useOpenGL;
    mSettings->setValue(KEY_USE_OPENGL, mUseOpenGL);

    emit useOpenGLChanged(mUseOpenGL);
}

void Preferences::setShowMiniMap(bool show)
{
    if (mShowMiniMap == show)
        return;
    mShowMiniMap = show;
    mSettings->setValue(KEY_SHOW_MINIMAP, show);
    emit showMiniMapChanged(mShowMiniMap);
}

void Preferences::setMiniMapWidth(int width)
{
    width = qMin(width, MINIMAP_WIDTH_MAX);
    width = qMax(width, MINIMAP_WIDTH_MIN);

    if (mMiniMapWidth == width)
        return;
    mMiniMapWidth = width;
    mSettings->setValue(KEY_MINIMAP_WIDTH, width);
    emit miniMapWidthChanged(mMiniMapWidth);
}

void Preferences::setBackgroundColor(const QColor &bgColor)
{
    if (mBackgroundColor == bgColor)
        return;

    mBackgroundColor = bgColor;
    mSettings->setValue(KEY_BG_COLOR, mBackgroundColor.name());
    emit backgroundColorChanged(mBackgroundColor);
}

void Preferences::setShowTileGrid(bool showGrid)
{
    if (showGrid == mShowTileGrid)
        return;
    mShowTileGrid = showGrid;
    mSettings->setValue(KEY_SHOW_TILE_GRID, mShowTileGrid);
    emit showTileGridChanged(mShowTileGrid);
}

void Preferences::setTileGridColor(const QColor &gridColor)
{
    if (gridColor == mTileGridColor)
        return;
    mTileGridColor = gridColor;
    mSettings->setValue(KEY_TILE_GRID_COLOR, mTileGridColor);
    emit tileGridColorChanged(mTileGridColor);
}

void Preferences::setTilesDirectory(const QString &path)
{
    if (mTilesDirectory == path)
        return;
    mTilesDirectory = path;
    mSettings->setValue(QLatin1String("TilesDirectory"), path);
    emit tilesDirectoryChanged();
}

void Preferences::addRecentFile(const QString &fileName)
{
    // Remember the file by its canonical file path
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    if (canonicalFilePath.isEmpty())
        return;

    const int MaxRecentFiles = 10;

    QStringList files = recentFiles();
    files.removeAll(canonicalFilePath);
    files.prepend(canonicalFilePath);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    mSettings->setValue(KEY_RECENT_FILES, files);
    emit recentFilesChanged();
}

QStringList Preferences::recentFiles() const
{
    return mSettings->value(KEY_RECENT_FILES).toStringList();
}
