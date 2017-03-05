#include "afrtable.h"
#include "ui_afrtable.h"
#include "tmodels.h"
#include "utilities.h"
#include "signals.h"
#include <QFileDialog>
#include <QDir>

AFRTABLE::AFRTABLE(QWidget *parent, Signals* transceiver) :
    QMainWindow(parent),
    ui(new Ui::AFRTABLE)
{
    ui->setupUi(this);
    setWindowTitle(QString("Air to Fuel Ratio Table"));
    setWindowIcon(QIcon(efidaq::DEFAULT_LOGO_FILEPATH));

    // Set to delete when closed.
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    // Initialize the model pointer.
    m_tmodel = new AFR_TABLE_MODEL(nullptr);
    if (m_tmodel->loadTable(efidaq::DEFAULT_AFR_TABLE_FILEPATH))
    {
        ui->afrTableView->setModel(m_tmodel);
    }

    this->transceiver = transceiver;

    // Connect the load table action to the loadTable function.
    connect(ui->actionLoad_Table, SIGNAL(triggered()), SLOT(loadTable()));

    // Connect the send and receive table buttons.
    connect(ui->updateControllerButton, SIGNAL(clicked()), SLOT(handle_updateControllerButton_clicked()));
    connect(ui->updateTableButton, SIGNAL(clicked()), SLOT(handle_updateTableButton_clicked()));

}

AFRTABLE::~AFRTABLE()
{
    delete ui;
    if (m_tmodel != nullptr)
    {
        delete m_tmodel;
    }
}

void AFRTABLE::loadTable()
{
    // Get a selected file from the user.
    QString filename = QFileDialog::getOpenFileName(this, "Select a file to open.", QDir::currentPath());

    // Check for cancellation
    if(filename.isEmpty())
    {
        return;
    }

    // Create the table and populate it with the values in the specified file.
    if (m_tmodel == nullptr)
    {
        m_tmodel = new AFR_TABLE_MODEL(nullptr);
    }
    else
    {
        delete m_tmodel;
        m_tmodel = new AFR_TABLE_MODEL(nullptr);
    }
    if (m_tmodel->loadTable(filename))
    {
        ui->afrTableView->setModel(m_tmodel);
    }
    else
    {
        notify("Failed to load specified file.");
    }
}

void AFRTABLE::handle_updateControllerButton_clicked()
{
    transceiver->sendTable(changedCellVals);
}

void AFRTABLE::handle_updateTableButton_clicked()
{
    transceiver->receiveTable(changedCells);
}
