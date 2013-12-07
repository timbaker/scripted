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

#include "mainwindow.h"
#include <QApplication>

#include "documentmanager.h"
#include "luamanager.h"
#include "metaeventmanager.h"
#include "node.h"
#include "preferences.h"
#include "progress.h"
#include "scriptmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("TheIndieStone"));
    a.setApplicationName(QLatin1String("PZDraft"));
#ifdef BUILD_INFO_VERSION
    a.setApplicationVersion(QLatin1String(AS_STRING(BUILD_INFO_VERSION)));
#else
    a.setApplicationVersion(QLatin1String("0.0.1"));
#endif

#ifdef Q_WS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    qRegisterMetaType<BaseNode*>("BaseNode*");

    new Preferences;
    new DocumentManager;
    new ScriptManager;

    new LuaManager;
//    luamgr()->readLuaFiles();

    new MetaEventManager;
    eventmgr()->readEventFiles();

    MainWindow w;
    w.show();
    w.readSettings();

    Progress::instance()->setMainWindow(&w);

    w.openLastFiles();

    return a.exec();
}
