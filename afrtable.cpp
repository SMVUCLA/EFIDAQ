#include "afrtable.h"
#include "ui_afrtable.h"
#include "tmodels.h"
#include "utilities.h"
#include "signals.h"
#include <QVector>
#include <QFileDialog>
#include <QDir>

// TABLE CRASHING WHEN TRYING TO UPDATE WITH CONTROLLER PARAMETERS

AFRTABLE::AFRTABLE(QWidget *parent, Signals* transceiver) :
    QMainWindow(parent),
    ui(new Ui::AFRTABLE)
{
    ui->setupUi(this);
    setWindowTitle(QString("Air to Fuel Ratio Table"));
    setWindowIcon(QIcon(efidaq::DEFAULT_LOGO_FILEPATH));

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

// Reimplemented events
void AFRTABLE::closeEvent(QCloseEvent *event)
{
    // want to ask to save table before exiting
    emit closing();
    event->accept();
}

// Other functions
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
bool AFRTABLE::saveTable()
{
    return false;
}

// Clicked pushButton handlers
void AFRTABLE::handle_updateControllerButton_clicked()
{
    if (transceiver->sendTable(m_tmodel->getChangedCells()) != 0)
    {
        notify("One or more cells failed to send.");
    }
    else
    {
        notify("Controller successfully updated.");
    }
}
void AFRTABLE::handle_updateTableButton_clicked()
{
    for (int row = 0; row < m_tmodel->getTable().length(); row++)
    {
        for (int col = 0; col < m_tmodel->getTable()[row].length(); col++)
        {
            QVector<int> coord;
            coord.append(row);
            coord.append(col);
            requestedCells.append(coord);
        }
    }
    QVector<float> vals = transceiver->receiveTable(requestedCells);
    for (int i = 0; i < vals.length(); i++)
    {
        m_tmodel->setVal(requestedCells, vals);
    }
}

// Menu action handlers
void AFRTABLE::handle_actionSaveAs()
{
    // needs to save
}
void AFRTABLE::handle_actionLoad_Table()
{
    // needs to load a table
}

