#ifndef CONNECTIONSDIALOG_H
#define CONNECTIONSDIALOG_H

#include "editor_global.h"
#include <QDialog>

namespace Ui {
class ConnectionsDialog;
}

class ConnectionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsDialog(ProjectDocument *doc, BaseNode *node,
                               const QString &outputName, QWidget *parent = 0);
    ~ConnectionsDialog();

private slots:
    void selectionChanged();
    void moveUp();
    void moveDown();
    void remove();
    void syncUI();

private:
    void setConnectionsTable();
    NodeConnection *prev();
    NodeConnection *next();

private:
    Ui::ConnectionsDialog *ui;
    BaseNode *mNode;
    QString mOutputName;
    NodeConnection *mCurrentCxn;
};

#endif // CONNECTIONSDIALOG_H
