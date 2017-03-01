#include "mainruntest.h"
#include "ui_mainruntest.h"
#include "serialreader.h"
#include "runtest.h"
#include "utilities.h"
#include "qformdialog.h"

#include <QMessageBox>
#include <QAbstractButton>
#include <QCloseEvent>
#include <QPushButton>
#include <QSet>
#include <QSizePolicy>
#include <QInputDialog>

MAINRUNTEST::MAINRUNTEST(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MAINRUNTEST)
{
    ui->setupUi(this);
    setWindowTitle(QString("EFI Data Acquisition System"));
    setWindowIcon(QIcon(efidaq::DEFAULT_LOGO_FILEPATH));

    // Delete when closed
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    // Create the RUNTEST widget.
    rt = new RUNTEST(this, this);

    // Set the RUNTEST widget as the central widget for the MAINRUNTEST object.
    setCentralWidget(rt);

    // Connect Save As menu action to the SaveData function.
    connect(ui->actionSave_As, SIGNAL(triggered()), SLOT(saveData()));

    // Connect Clear menu action to the clearData function.
    connect(ui->actionClear, SIGNAL(triggered()), SLOT(clearData()));

    // Connect Locked menu checkbox action to the dataLockChanged function
    connect(ui->actionLocked, SIGNAL(toggled(bool)), SLOT(dataLockChanged(bool)));

    // Connect the parameter edit menu action to its handler function.
    connect(ui->actionEdit, SIGNAL(triggered()), SLOT(handleParametersEditTriggered()));

    // Connect the collection protocol menu actions to their handler functions.
    connect(ui->actionProtocolSerialBlocks, SIGNAL(triggered(bool)), SLOT(handleProtocolSerialBlocksTriggered(bool)));
    connect(ui->actionProtocolUDP_Packets, SIGNAL(triggered(bool)), SLOT(handleProtocolUDPPacketsTriggered(bool)));

    // Connect command window action
    connect(ui->actionCommand_Window, SIGNAL(triggered()), SLOT(handleActionCommand_Window_triggered()));
}

MAINRUNTEST::~MAINRUNTEST()
{
    delete ui;
    delete rt;
}

bool MAINRUNTEST::saveData()
{
    switch (rt->saveData())
    {
    case efidaq::SUCCESS:
        notify("Data successfully saved.");
        return true;
    case efidaq::CANCELLED:
        notify("Saving cancelled.");
        return false;
    case efidaq::FAILED_CURRENTLY_COLLECTING:
        notify("Data could not be saved. Make sure the system is not currently collecting data.");
        return false;
    case efidaq::OPEN_FILE_FAILED:
        notify("Data could not be saved. Failed to open the specified file for writing.");
        return false;
    case efidaq::WRITE_FILE_FAILED:
        notify("Data could not be saved. Failed to write to specified file.");
        return false;
    default:
        notify("ERROR: Unspecified return value.");
        return false;
    }
}

bool MAINRUNTEST::clearData()
{
    switch (rt->clearData())
    {
    case efidaq::SUCCESS:
        notify("Data successfully cleared.");
        return true;
    case efidaq::CANCELLED:
        notify("Clear command cancelled.");
        return false;
    case efidaq::FAILED_CURRENTLY_COLLECTING:
        notify("Data could not be cleared. Make sure the system is not currently collecting data.");
        return false;
    default:
        notify("ERROR: Unspecified return value.");
        return false;
    }
}

void MAINRUNTEST::dataLockChanged(bool checked)
{
    rt->setDataLocked(checked);
}

void MAINRUNTEST::closeEvent(QCloseEvent *event)
{
    QMessageBox msgbox;
    msgbox.setText("Would you like to save the current data before exiting?");
    QAbstractButton* a = msgbox.addButton(QMessageBox::Yes);
    QAbstractButton* b = msgbox.addButton(QMessageBox::No);
    msgbox.addButton(QMessageBox::Cancel);
    msgbox.exec();
    QAbstractButton* d = msgbox.clickedButton();
    if (rt->isCollectingData())
    {
        rt->on_EndDCButton_clicked();
    }
    if (d == a)
    {
        if (saveData())
        {
            event->accept();
        }
        else
        {
            QMessageBox msgbox2;
            msgbox2.setText("Would you still like to exit?");
            QAbstractButton* a = msgbox2.addButton(QMessageBox::Yes);
            msgbox2.addButton(QMessageBox::No);
            msgbox2.exec();
            QAbstractButton* d = msgbox2.clickedButton();
            if (d == a)
            {
                event->accept();
            }
            else
            {
                event->ignore();
            }
        }
    }
    else if (d == b)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MAINRUNTEST::handleParametersEditTriggered()
{
    QFormDialog pform(this);
    pform.addPair(QString("a"), QString("b"));
    pform.addPair(QString("c"), QString("d"));
    pform.addPair(QString("e"), QString("f"));
    pform.addPair(QString("g"), QString("h"));
    pform.exec();
    QSet<std::pair<QString, QString>> results = pform.getPairs();
    QFormDialog pform2(this);
    for (auto it = results.begin(); it != results.end(); it++)
    {
        pform2.addPair(*it);
    }
    pform2.exec();
}

void MAINRUNTEST::resizeEvent(QResizeEvent* event)
{
    if (rt->isCollectingData())
    {
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

void MAINRUNTEST::setResizeable(bool resizeable)
{
    if (resizeable)
    {
        setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        setMinimumSize(0,0);
        ui->statusbar->setSizeGripEnabled(true);
    }
    else
    {
        this->setFixedSize(size());
        ui->statusbar->setSizeGripEnabled(false);
    }
}

bool MAINRUNTEST::isFilteringByNumFields() const
{
    return ui->actionFilterByNumFields->isChecked();
}

bool MAINRUNTEST::isFilteringByContent() const
{
    return ui->actionFilterByContent->isChecked();
}

// Intended to toggle between both protocols. Must always be at least one protocol.
void MAINRUNTEST::handleProtocolSerialBlocksTriggered(bool checked)
{
    if (!rt->isCollectingData())
    {
        ui->actionProtocolUDP_Packets->setChecked(!checked);
        ui->actionProtocolSerialBlocks->setChecked(checked);
        if (checked)
        {
            notify("Collection through Serial Blocks activated.");
        }
        else
        {
            notify("Collection through UDP Packets activated.");
        }
    }
    else
    {
        ui->actionProtocolSerialBlocks->setChecked(!checked);
        notify("Collection protocol cannot be changed while data is being collected.");
    }
}

void MAINRUNTEST::handleProtocolUDPPacketsTriggered(bool checked)
{
    if (!rt->isCollectingData())
    {
        ui->actionProtocolSerialBlocks->setChecked(!checked);
        ui->actionProtocolUDP_Packets->setChecked(checked);
        if (checked)
        {
            notify("Collection through UDP Packets activated.");
        }
        else
        {
            notify("Collection through Serial Blocks activated.");
        }
    }
    else
    {
        ui->actionProtocolUDP_Packets->setChecked(!checked);
        notify("Collection protocol cannot be changed while data is being collected.");
    }
}

int MAINRUNTEST::collectionMethod() const
{
    if (ui->actionProtocolSerialBlocks->isChecked())
    {
        return efidaq::COLLECTION_BY_SERIAL;
    }
    else if (ui->actionProtocolUDP_Packets->isChecked())
    {
        return efidaq::COLLECTION_BY_UDP;
    }
    else
    {
        return efidaq::COLLECTION_BY_SERIAL;
    }
}

bool MAINRUNTEST::isShowingValues() const
{
    return ui->actionShow_Values->isChecked();
}

bool MAINRUNTEST::isAutoScrolling() const
{
    return ui->actionAutoScroll->isChecked();
}


void MAINRUNTEST::handleActionCommand_Window_triggered()
{
    QString entry = QInputDialog::getText(nullptr, "Send Commands to Controller","");
    Signals* transceiver = rt->getTransceiver();
    if(entry == "4")
    {
        QVector<QVector<float>> rowCols;
        QVector<float> cell;
        cell.append(3);
        cell.append(4);
        cell.append(17);
        rowCols.append(cell);
        int ret = transceiver->sendTable(rowCols);
        if (ret > 0)
        {
            notify("GOT NOTHING BACK!");
        }
        else
        {
            notify(QString("Sent"));
        }
    }
    if(entry == "5")
    {
        QVector<QVector<int>> rowCols;
        QVector<int> cell;
        cell.append(3);
        cell.append(4);
        rowCols.append(cell);
        QVector<float> vals = transceiver->receiveTable(rowCols);
        if (vals.length() == 0)
        {
            notify("GOT NOTHING BACK!");
        }
        else
        {
            notify(QString("Value received: %1").arg(vals[0]));
        }
    }
}
