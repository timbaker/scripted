#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "mainwindow.h"
#include "preferences.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    connect(ui->gameDirList, SIGNAL(itemSelectionChanged()), SLOT(syncUI()));
    connect(ui->addGameDir, SIGNAL(clicked()), SLOT(addGameDir()));
    connect(ui->removeGameDir, SIGNAL(clicked()), SLOT(removeGameDir()));

    foreach (QString f, prefs()->gameDirectories())
        ui->gameDirList->addItem(QDir::toNativeSeparators(f));

    syncUI();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::addGameDir()
{
    QString f = QFileDialog::getExistingDirectory(mainwin(), tr("Choose Game/MOD Directory"));
    if (!f.isEmpty()) {
        ui->gameDirList->addItem(QDir::toNativeSeparators(f));
    }
}

void PreferencesDialog::removeGameDir()
{
    QList<QListWidgetItem*> selected = ui->gameDirList->selectedItems();
//    int row = ui->gameDirList->row(selected.first());
    delete selected.first();
}

void PreferencesDialog::syncUI()
{
    ui->removeGameDir->setEnabled(ui->gameDirList->selectedItems().size() != 0);
}

void PreferencesDialog::accept()
{
    QStringList dirList;
    for (int row = 0; row < ui->gameDirList->count(); row++) {
        QListWidgetItem *item = ui->gameDirList->item(row);
        dirList += item->text();
    }
    prefs()->setGameDirectories(dirList);

    QDialog::accept();
}
