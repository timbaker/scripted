#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

private slots:
    void addGameDir();
    void removeGameDir();
    void syncUI();

    void accept();

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
