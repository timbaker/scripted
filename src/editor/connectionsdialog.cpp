#include "connectionsdialog.h"
#include "ui_connectionsdialog.h"

#include "node.h"
#include "projectactions.h"
#include "projectdocument.h"
#include "undoredobuttons.h"

ConnectionsDialog::ConnectionsDialog(ProjectDocument *doc, BaseNode *node,
                                     const QString &outputName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionsDialog),
    mNode(node),
    mOutputName(outputName),
    mCurrentCxn(0)
{
    ui->setupUi(this);

    UndoRedoButtons *urb = new UndoRedoButtons(doc->undoStack(), this);
    ui->buttonsLayout->insertWidget(0, urb->redoButton());
    ui->buttonsLayout->insertWidget(0, urb->undoButton());

    connect(ui->connectionsTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));
    connect(ui->moveUp, SIGNAL(clicked()), SLOT(moveUp()));
    connect(ui->moveDown, SIGNAL(clicked()), SLOT(moveDown()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(remove()));

    setConnectionsTable();
    ui->connectionsTable->setOutputName(outputName);

    ui->connectionsTable->header()->resizeSection(0, 250);

    syncUI();
}

ConnectionsDialog::~ConnectionsDialog()
{
    delete ui;
}

void ConnectionsDialog::setConnectionsTable()
{
    NodeConnection *cxn = mCurrentCxn;
    ui->connectionsTable->setNode(mNode);
    if (mNode->connectionCount()) {
        ui->connectionsTable->selectConnection(cxn);
    }
}

NodeConnection *ConnectionsDialog::prev()
{
    if (!mCurrentCxn) return 0;
    for (int i = mNode->indexOf(mCurrentCxn) - 1; i >= 0; i--) {
        NodeConnection *cxn = mNode->connection(i);
        if (cxn->mOutput == mOutputName)
            return cxn;
    }
    return 0;
}

NodeConnection *ConnectionsDialog::next()
{
    if (!mCurrentCxn) return 0;
    for (int i = mNode->indexOf(mCurrentCxn) + 1; i < mNode->connectionCount(); i++) {
        NodeConnection *cxn = mNode->connection(i);
        if (cxn->mOutput == mOutputName)
            return cxn;
    }
    return 0;
}

void ConnectionsDialog::selectionChanged()
{
    mCurrentCxn = 0;
    QModelIndexList selected = ui->connectionsTable->selectionModel()->selectedRows();
    if (selected.size() == 1) {
        int row = selected.first().row();
        mCurrentCxn = mNode->connection(row);
    }

    syncUI();
}

void ConnectionsDialog::moveUp()
{
    int from = mNode->indexOf(mCurrentCxn);
    int to = mNode->connections().indexOf(prev());
    ProjectActions::instance()->reorderConnection(mNode, from, to);
}

void ConnectionsDialog::moveDown()
{
    int from = mNode->indexOf(mCurrentCxn);
    int to = mNode->connections().indexOf(next());
    ProjectActions::instance()->reorderConnection(mNode, from, to);
}

void ConnectionsDialog::remove()
{
    ProjectActions::instance()->removeConnection(mNode, mCurrentCxn);
}

void ConnectionsDialog::syncUI()
{
    ui->moveUp->setEnabled(prev() != 0);
    ui->moveDown->setEnabled(next() != 0);
    ui->remove->setEnabled(mCurrentCxn != 0);
}
