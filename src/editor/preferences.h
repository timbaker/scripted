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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "singleton.h"

#include <QColor>
#include <QObject>

class QSettings;

class Preferences : public QObject, public Singleton<Preferences>
{
    Q_OBJECT
public:
    Preferences();
    ~Preferences();

    QString configPath() const
    { return mConfigDirectory; }
    QString configPath(const QString &fileName) const;

    QString appConfigPath() const;
    QString appConfigPath(const QString &fileName) const;

    void setScriptsDirectory(const QString &path);
    QString scriptsDirectory() const
    { return mScriptsDirectory; }

    void setGameDirectories(const QStringList &dirList);
    QStringList gameDirectories() const
    { return mGameDirectories; }

    bool useOpenGL() const
    { return mUseOpenGL; }
    
    bool showMiniMap() const
    { return mShowMiniMap; }
#define MINIMAP_WIDTH_MIN 128
#define MINIMAP_WIDTH_MAX 512
    void setMiniMapWidth(int width);
    int miniMapWidth() const
    { return mMiniMapWidth; }

    QColor backgroundColor() const
    { return mBackgroundColor; }

    bool snapToGrid() const
    { return mSnapToGrid; }

    bool showTileGrid() const
    { return mShowTileGrid; }

    QColor tileGridColor() const
    { return mTileGridColor; }

    QString tilesDirectory() const
    { return mTilesDirectory; }

    void addRecentFile(const QString &fileName);
    QStringList recentFiles() const;

signals:
    void scriptsDirectoryChanged();
    void gameDirectoriesChanged();
    void useOpenGLChanged(bool opengl);
    void showMiniMapChanged(bool show);
    void miniMapWidthChanged(int width);
    void backgroundColorChanged(const QColor &color);
    void showTileGridChanged(bool showGrid);
    void tileGridColorChanged(const QColor &color);
    void tilesDirectoryChanged();
    void recentFilesChanged();

public slots:
    void setUseOpenGL(bool useOpenGL);
    void setShowMiniMap(bool show);
    void setBackgroundColor(const QColor &bgColor);
    void setShowTileGrid(bool showGrid);
    void setTileGridColor(const QColor &gridColor);
    void setTilesDirectory(const QString &path);

private:
    QSettings *mSettings;
    QString mScriptsDirectory;
    bool mUseOpenGL;
    bool mShowMiniMap;
    int mMiniMapWidth;
    QColor mBackgroundColor;
    bool mShowTileGrid;
    bool mSnapToGrid;
    QColor mTileGridColor;
    QString mConfigDirectory;
    QString mTilesDirectory;
    QStringList mGameDirectories;
};

inline Preferences *prefs() { return Preferences::instance(); }

#endif // PREFERENCES_H
