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

#include "projectactions.h"

#include "documentmanager.h"
#include "node.h"
#include "mainwindow.h"
#include "nodepropertiesdialog.h"
#include "preferences.h"
#include "progress.h"
#include "project.h"
#include "projectdocument.h"
#include "projectreader.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

static const QLatin1String KEY_OPEN_PROJECT_DIRECTORY("OpenProjectDirectory");
static const QLatin1String KEY_SAVE_PROJECT_DIRECTORY("SaveProjectDirectory");

SINGLETON_IMPL(ProjectActions)

ProjectActions::ProjectActions(Ui::MainWindow *actions, QObject *parent) :
    QObject(parent),
    mActions(actions)
{
    connect(mActions->actionNewProject, SIGNAL(triggered()), SLOT(newProject()));
    connect(mActions->actionOpenProject, SIGNAL(triggered()), SLOT(openProject()));
    connect(mActions->actionSaveProject, SIGNAL(triggered()), SLOT(saveProject()));
    connect(mActions->actionSaveProjectAs, SIGNAL(triggered()), SLOT(saveProjectAs()));
    connect(mActions->actionQuit, SIGNAL(triggered()), MainWindow::instance(), SLOT(close()));

    connect(mActions->actionPreferences, SIGNAL(triggered()), SLOT(preferencesDialog()));
}

ProjectDocument *ProjectActions::document()
{
    if (Document *doc = docman()->currentDocument())
        return doc->asProjectDocument();
    return 0;
}

void ProjectActions::newProject()
{
    Project *prj = new Project();
    ProjectDocument *doc = new ProjectDocument(prj, QString());
    docman()->addDocument(doc);
}

void ProjectActions::openProject()
{
    QSettings settings;

    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = tr("PZDraft project (*.pzs)");
    filter += selectedFilter;

    QStringList fileNames =
            QFileDialog::getOpenFileNames(MainWindow::instance(), tr("Open Project"),
                                          settings.value(KEY_OPEN_PROJECT_DIRECTORY).toString(),
                                          filter, &selectedFilter);
    if (fileNames.isEmpty())
        return;

    settings.setValue(KEY_OPEN_PROJECT_DIRECTORY, QFileInfo(fileNames[0]).absolutePath());

    foreach (const QString &fileName, fileNames)
        openProject(fileName);
}

void ProjectActions::openProject(const QString &fileName)
{
    int n = docman()->findDocument(fileName);
    if (n != -1) {
        docman()->setCurrentDocument(n);
//        MainWindow::instance()->switchToEditMode();
        return;
    }

    QFileInfo fileInfo(fileName);
    PROGRESS progress(tr("Reading %1").arg(fileInfo.fileName()));

    ProjectReader reader;
    Project *project = reader.read(fileName);
    if (!project) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Reading Project"),
                              reader.errorString());
        return;
    }

#if 0
    foreach (BaseNode *node, project->rootNode()->nodes()) {
        if (!node->mDefinition)
            continue;
        foreach (DraftProperty *p, node->mDefinition->mProperties) {
            if (node->variable(p->mName) == NULL) {
                DraftProperty *np = new DraftProperty(p);
                node->insertVariable(node->variableCount(), np);
            }
        }
    }
#endif

    docman()->addDocument(new ProjectDocument(project, fileName));
    if (docman()->failedToAdd())
        return;

    prefs()->addRecentFile(fileName);
}

bool ProjectActions::saveProject()
{
    QString fileName = document()->fileName();
    if (!fileName.isEmpty())
        return saveProject(fileName);
    return saveProjectAs();
}

bool ProjectActions::saveProject(const QString &fileName)
{
    QString error;
    if (!document()->save(fileName, error)) {
        QMessageBox::critical(MainWindow::instance(), tr("Error Saving Project"), error);
        return false;
    }
    prefs()->addRecentFile(fileName);
    return true;
}

bool ProjectActions::saveProjectAs()
{
    QSettings settings;

    QString suggestedFileName;
    if (!document()->fileName().isEmpty()) {
        const QFileInfo fileInfo(document()->fileName());
        suggestedFileName = fileInfo.path();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += fileInfo.completeBaseName();
        suggestedFileName += QLatin1String(".pzs");
    } else {
        QString path = settings.value(KEY_SAVE_PROJECT_DIRECTORY).toString();
        if (path.isEmpty() || !QDir(path).exists())
            path = QDir::currentPath();
        suggestedFileName = path;
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += tr("untitled.pzs");
    }

    const QString fileName =
            QFileDialog::getSaveFileName(MainWindow::instance(), QString(), suggestedFileName,
                                         tr("PZDraft project (*.pzs)"));
    if (!fileName.isEmpty()) {
        settings.setValue(KEY_SAVE_PROJECT_DIRECTORY, QFileInfo(fileName).absolutePath());
        return saveProject(fileName);
    }
    return false;
}

void ProjectActions::preferencesDialog()
{

}

void ProjectActions::nodePropertiesDialog(BaseNode *node)
{
    NodePropertiesDialog d(MainWindow::instance());
    d.setObject(node);
    d.exec();
}

void ProjectActions::updateActions()
{

}
