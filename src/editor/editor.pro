include($$top_srcdir/scripted.pri)
include(../lua/lua.pri)
#include(../qtpropertybrowser/src/qtpropertybrowser.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = editor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    welcomemode.cpp \
    simplefile.cpp \
    imode.cpp \
    fancytabwidget.cpp \
    embeddedmainwindow.cpp \
    basegraphicsview.cpp \
    basegraphicsscene.cpp \
    toolmanager.cpp \
    preferences.cpp \
    zoomable.cpp \
    projectactions.cpp \
    utils/stylehelper.cpp \
    utils/styledbar.cpp \
    documentmanager.cpp \
    document.cpp \
    progress.cpp \
    projectreader.cpp \
    project.cpp \
    projectdocument.cpp \
    projectwriter.cpp \
    editmode.cpp \
    abstracttool.cpp \
    projectchanger.cpp \
    projecttreeview.cpp \
    projecttreedock.cpp \
    scriptvariablesview.cpp \
    scriptvariable.cpp \
    scriptvariablesdock.cpp \
    scriptsdock.cpp \
    node.cpp \
    nodeitem.cpp \
    scriptview.cpp \
    scriptscene.cpp \
    luamanager.cpp \
    luadockwidget.cpp \
    nodepropertiesdialog.cpp \
    nodepropertieslist.cpp \
    nodeconnectionslist.cpp \
    scriptmanager.cpp \
    filesystemwatcher.cpp \
    variablepropertiesdialog.cpp \
    undoredobuttons.cpp \
    scenescriptdialog.cpp \
    editnodevariabledialog.cpp \
    metaeventmanager.cpp \
    metaeventdock.cpp

HEADERS  += mainwindow.h \
    scriptscene.h \
    welcomemode.h \
    singleton.h \
    simplefile.h \
    imode.h \
    fancytabwidget.h \
    embeddedmainwindow.h \
    basegraphicsview.h \
    basegraphicsscene.h \
    editor_global.h \
    toolmanager.h \
    preferences.h \
    zoomable.h \
    projectactions.h \
    utils/stylehelper.h \
    utils/styledbar.h \
    utils/hostosinfo.h \
    documentmanager.h \
    document.h \
    progress.h \
    projectreader.h \
    project.h \
    projectdocument.h \
    projectwriter.h \
    editmode.h \
    abstracttool.h \
    projectchanger.h \
    projecttreeview.h \
    projecttreedock.h \
    scriptvariablesview.h \
    scriptvariable.h \
    scriptvariablesdock.h \
    scriptsdock.h \
    node.h \
    nodeitem.h \
    scriptscene.h \
    scriptview.h \
    luamanager.h \
    luadockwidget.h \
    nodepropertiesdialog.h \
    nodeconnectionslist.h \
    nodepropertieslist.h \
    scriptmanager.h \
    filesystemwatcher.h \
    variablepropertiesdialog.h \
    undoredobuttons.h \
    scenescriptdialog.h \
    editnodevariabledialog.h \
    metaeventmanager.h \
    metaeventdock.h

FORMS    += mainwindow.ui \
    welcomemode.ui \
    projecttreedock.ui \
    scriptvariablesdock.ui \
    scriptsdock.ui \
    luadockwidget.ui \
    nodepropertiesdialog.ui \
    variablepropertiesdialog.ui \
    scenescriptdialog.ui \
    editnodevariabledialog.ui \
    metaeventdock.ui

RESOURCES += \
    editor.qrc
