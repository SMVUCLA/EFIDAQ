#include "runtest.h"
#include "ui_runtest.h"
#include "serialreader.h"
#include "udpreader.h"
#include "tmodels.h"
#include "plotwindow.h"
#include "utilities.h"
#include "afrtable.h"
#include "mainruntest.h"
#include "datainterpreter.h"

#include <QTimer>
#include <QModelIndex>
#include <QMessageBox>
#include <QPushButton>
#include <QVector>
#include <QFileDialog>
#include <QDir>
#include <QFile>

// Constructor
RUNTEST::RUNTEST(MAINRUNTEST* mrtparent, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::RUNTEST)
{
    // Sets up the window to look like its ".ui" file.
    ui->setupUi(this);

    // Set the MAINRUNTEST parent
    this->mrtparent = mrtparent;

    // Initialize the number of data points collected as 0.
    m_ndp = 0;

    // Locks the data stream from being scrolled through or edited.
    ui->DataBrowser->setDisabled(true);

    // Set up the syntax highlighter for the DataBrowser
    highlighter = new Highlighter(ui->DataBrowser->document());

    // Sets the xlistView to the LIST_CHOICES_MODEL.
    m_xlmodel = new LIST_CHOICES_MODEL(this);
    ui->xlistView->setModel(m_xlmodel);
    connect(ui->xlistView, SIGNAL(clicked(QModelIndex)), SLOT(xItemChanged(QModelIndex)));
    connect(ui->xlistView, SIGNAL(activated(QModelIndex)), SLOT(xItemChanged(QModelIndex)));
    // Initializes the default axes labels by taking the elements at the first index
    // in the model. If no element exists, defaults to an empty label.
    m_xLabel.second = 0;
    if (!m_xlmodel->getItemAt(0, m_xLabel.first))
    {
        m_xLabel.first = "";
    }

    // Sets the ylistView to the LIST_CHOICES_MODEL.
    m_ylmodel = new LIST_CHOICES_MODEL(this);
    ui->ylistView->setModel(m_ylmodel);
    connect(ui->ylistView, SIGNAL(clicked(QModelIndex)), SLOT(yItemChanged(QModelIndex)));
    connect(ui->ylistView, SIGNAL(activated(QModelIndex)), SLOT(yItemChanged(QModelIndex)));
    // Initializes the default axes labels by taking the elements at the first index
    // in the model. If no element exists, defaults to an empty label.
    m_yLabel.second = 0;
    if (!m_ylmodel->getItemAt(0, m_yLabel.first))
    {
        m_yLabel.first = "";
    }
    expectedNumFields = m_ylmodel->rowCount();

    // Sets the max and minimum values on the slider
    // This is actually the maximum sample rate speed using the current method.
    // We will need more advanced programming to get the sample rate higher than
    // 40 samples per second.
    ui->sampleRateSlider->setMaximum(1000);
    ui->sampleRateSlider->setMinimum(1);

    // Sets precision on slider.
    ui->sampleRateSlider->setTickInterval(1);

    // Sets up a time that is supposed to update the textbrowser with any new
    // text in the DataBrowser source document, however, cannot get this to
    // work. May need to implement different method for updating the text
    // browser.
    m_dataRefrTimer = new QTimer(this);
    connect(&m_streamRefrTimer, SIGNAL(timeout()), this, SLOT(handleStreamRefrTimeout()));
    m_streamRefrTimer.start(33);

    // This kind of timer is automatically repeating.
    // The interval is in milliseconds.
    m_dataRefrTimer->setInterval( (int) ((((double)1/ui->sampleRateSlider->minimum())) * 1000));

    // Connects the timer's "timeout()" signal to the specified slot, which is
    // a function of the RUNTEST class.
    connect(m_dataRefrTimer, SIGNAL(timeout()), this, SLOT(hitDataTimer()));

    // Initialize the End Data Collection Button as disable
    ui->EndDCButton->setDisabled(true);

    // Initialize serial reader for taking in data
    m_serialReader = new SERIALREADER(this);
    ui->serialPortNameLabel->setText(m_serialReader->portName());

    // Initialize udp reader for taking in data
    m_udpReader = new UDPReader(this);
    m_udpReader->setIPAddress(efidaq::DEFAULT_IPADDRESS);
    m_udpReader->setPortNo(efidaq::DEFAULT_PORTNO);
    ui->IPEdit->setText(m_udpReader->getIPAddressString());
    ui->PortEdit->setText(QString("%1").arg(m_udpReader->getPortNo()));
    connect(ui->IPEdit, SIGNAL(editingFinished()), this, SLOT(handleIPAddressEditFinished()));
    connect(ui->PortEdit, SIGNAL(editingFinished()), this, SLOT(handlePortNoEditFinished()));

    // Allow the serial reader to send a stop collection signal in the event of
    // an abrupt connection break.
    connect(m_serialReader, SIGNAL(stopCollecting()), SLOT(handle_connectionTerminated()));

    // Initialize the byte buffer for serial input
    m_bytebuf = new QByteArray;

    // Initialize the ByteFilter that will be used to filter out bad input.
    filter.addFilter(QString("[^0123456789.-,+\n]"));

    m_afrtable=nullptr;

    // Dynamically Allocate Signals class
    transceiver = new Signals(m_serialReader);

    connect(ui->triggerDataRefrTimer_button, SIGNAL(clicked(bool)), SLOT(handle_triggerDataRefrTimer_button()));
}

// Destructor for the RUNTEST class
RUNTEST::~RUNTEST()
{
    if (m_dataRefrTimer->isActive())
    {
        on_EndDCButton_clicked();
    }
    delete ui;
    delete m_xlmodel;
    delete m_ylmodel;
    delete m_dataRefrTimer;
    delete m_bytebuf;
    delete highlighter;
    if(m_afrtable!=nullptr)
        delete m_afrtable;

    for (auto it = pw.begin(); it != pw.end();)
    {
        delete *it;
        it = pw.erase(it);
    }

    //notify("Deleted RUNTEST");
}

// Stops plotting
void RUNTEST::stopPlotting(PlotWindow* sender)
{
    for (auto it = pw.begin(); it != pw.end();)
    {
        if (*it == sender)
        {
            delete *it;
            it = pw.erase(it);
            m_xData.pop_back();
            m_yData.pop_back();
        }
        else
        {
             it++;
        }
    }
    size_t count = 0;
    for (auto it = pw.begin(); it != pw.end(); it++)
    {
        (*it)->setWindowTitle(QString("Plot ID: %1").arg(count));
        count++;
    }
}

// Saves the current data in the DataBrowser.
// Requires the system to not currently be taking data.
int RUNTEST::saveData()
{
    if (m_dataRefrTimer->isActive())
    {
        return efidaq::FAILED_CURRENTLY_COLLECTING;
    }
    else
    {
        QDir dir;
        QString extension;
        QString filter= "Worksheet(*.csv);;Text(*.txt);;BitFile(*.bit)";
        QString filename = QFileDialog::getSaveFileName(this, QString("Select a file to save the data to."),
                                                        dir.currentPath(), filter, &extension);
        if (filename.isEmpty())
        {
            return efidaq::CANCELLED;
        }
        QFile file(filename, this);
        if (!file.open(QFileDevice::WriteOnly))
        {
            return efidaq::OPEN_FILE_FAILED;
        }
        if (extension == "BitFile(*.bit)")
        {
            if (file.write(buffer) == -1)
            {
                file.close();
                return efidaq::WRITE_FILE_FAILED;
            }
        }
        else
        {
            WriteInterpreter writer;
            QString fileContents("");
            char* bufferAddress = buffer.data();
            int i;
            //ui->DataBrowser->insertPlainText(QString("%1").arg(buffer.length())+"\n");
            for(i = 0; i < buffer.length()/writer.getNumBytes(); i++)
            {
                writer.setBytes(&(bufferAddress[i*writer.getNumBytes()]));
                fileContents += writer.getString() + "\n";
            }
            //ui->DataBrowser->insertPlainText(fileContents);
            QTextStream out(&file);
            out << fileContents;
//            if ((ret = file.write((const char*)fileContents.data())) == -1)
//            {
//                file.close();
//                return efidaq::WRITE_FILE_FAILED;
//            }
        }

        file.close();
        return efidaq::SUCCESS;
    }
}

// Clears the information in the DataBrowser.
// Requires the system to not currently be taking data.
int RUNTEST::clearData()
{
    if (m_dataRefrTimer->isActive())
    {
        return efidaq::FAILED_CURRENTLY_COLLECTING;
    }
    else
    {
        QMessageBox msgbox;
        msgbox.setText("Are you sure you want to clear the current data stream?");
        QAbstractButton* a = msgbox.addButton(QMessageBox::Yes);
        msgbox.addButton(QMessageBox::Cancel);
        msgbox.exec();
        QAbstractButton* b = msgbox.clickedButton();
        if (b == a)
        {
            ui->DataBrowser->clear();
            buffer.clear();
            m_ndp = 0;
            ui->NumDPlcd->display((int) m_ndp);
            ui->StartDCButton->setText("Start Data Collection");
            return efidaq::SUCCESS;
        }
        else
        {
            return efidaq::CANCELLED;
        }
    }
}

// Toggle the data browser lock.
// True means scrollable and editable.
// False means the opposite.
void RUNTEST::setDataLocked(bool yes)
{
    ui->DataBrowser->setDisabled(yes);
    ui->DataBrowser->setReadOnly(yes);
}

// Function that runs everytime the timer triggers.
// Reads available data and sends it to the graphs and terminal
// window to be displayed.
void RUNTEST::hitDataTimer()
{
    QByteArray data(*m_bytebuf);
    unsigned long long nBytes = 0;

    // Ask the main window about which collection method is being used.
    switch (mrtparent->collectionMethod())
    {
    case efidaq::COLLECTION_BY_SERIAL:
        nBytes = m_serialReader->availableData(data);
        break;
    case efidaq::COLLECTION_BY_UDP:
        nBytes = m_udpReader->availableData(data);
        break;
    default:
        return;
    }

    m_bytebuf->clear();

    // Check to see if any bytes were recorded
    if (nBytes <= 0 && data.isEmpty())
    {
        // If no bytes are available and there are no bytes waiting to be recorded,
        // then just return because there is no work to be done.

        //ui->DataBrowser->insertPlainText("No Data: "+QString("%1").arg(nBytes)+"\n\n");
        return;
    }

    //ui->DataBrowser->insertPlainText(QString("%1").arg(nBytes)+"\n\n");

    DataInterpreter interpreter;
    char* dataAddress = data.data();
    int numBytesPerMessage = interpreter.getNumBytes();
    int offset;
    bool validData;

    // Loops through the individual data points and prints them to the screen one at a time.
    for (offset = 0; offset <= data.length()- numBytesPerMessage; offset = offset+numBytesPerMessage)
    {
        // Alignment checking
        validData = false;
        while(!validData && offset <= data.length() - numBytesPerMessage)
        {
            interpreter.setBytes(&(dataAddress[offset]));
            if(interpreter.getEnd() == 0x80000000 && interpreter.getStart() == 0x80000001)
            {
                validData = true;
#if 0
                for(int j = 0; j < interpreter.getNumFields(); j++)
                {
                    if(interpreter.getValue(j) < 0)
                    {
                        offset++;
                        validData = false;
                        break;
                    }
                }
#endif
            }
            else
            {
                offset++;
            }
        }
        if(offset > data.length()-numBytesPerMessage)
        {
            break;
        }

        if (mrtparent->isShowingValues())
        {
            ui->DataBrowser->insertPlainText(interpreter.getString() + '\n');
        }
        buffer.append(&(((char*)interpreter.getBytes())[4]) , interpreter.getNumBytes()-8);


        // Appends the xData and yData points
        // Check is necessary to ensure index is within the range of data input.
        for (int i = 0; i < pw.size(); i++)
        {
            if (pw[i]->getXLabel().second < interpreter.getNumFields() && pw[i]->getYLabel().second < interpreter.getNumFields())
            {
                double xval = interpreter.getValue(pw[i]->getXLabel().second);
                m_xData[i].append(xval);
                //ui->DataBrowser->insertPlainText("\n\n"+QString("%1").arg(xval)+"\n\n");

                double yval = interpreter.getValue(pw[i]->getYLabel().second);
                m_yData[i].append(yval);
                //ui->DataBrowser->insertPlainText("\n\n"+QString("%1").arg(yval)+"\n\n");
            }
        }
        m_ndp++;

    }

    //ui->DataBrowser->insertPlainText("Append Info: Bytes = "+QString("%1").arg(data.length())+", Offset = "+QString("%1").arg(offset)+ "\n");

    if(offset == data.length()- numBytesPerMessage + 1)
    {
        m_bytebuf->append(&(dataAddress[offset - 1]), data.length()-(offset-1));
    }
    else if (data.length() - numBytesPerMessage + 1 < offset && offset < data.length())
    {
        m_bytebuf->append(&(dataAddress[offset]), data.length() - offset);
    }
    else
    {
        //This should only happen if data.length() = offset, in which case, there shoul be nothing to do, else there
        //is an error.

        //ui->DataBrowser->insertPlainText("Append Error: Bytes = "+QString("%1").arg(data.length())+", Offset = "+QString("%1").arg(offset)+ "\n");
    }


    // Update the LCD with the total sample number
    ui->NumDPlcd->display((int) m_ndp);

    // Update plots
    addData();
}

// Activates whenever the sampleRateSlider is moved.
void RUNTEST::on_sampleRateSlider_sliderMoved(int position)
{
    // Update the sampleRateEdit to the slider value
    ui->sampleRateEdit->setText(QString("%1").arg(position));

    // Update the timer interval to match the slider value.
    m_dataRefrTimer->setInterval( (int) ((((double)1/position)) * 1000));
}

// Activates whenever the sampleRateEdit has text entered.
void RUNTEST::on_sampleRateEdit_editingFinished()
{
    // Set the value of the slider to the value input into the sampleRateEdit
    // if the input in the edit is valid.
    QString text = ui->sampleRateEdit->text();
    int value = text.toInt();
    if (value >= ui->sampleRateSlider->minimum() && value <= ui->sampleRateSlider->maximum())
    {
        ui->sampleRateSlider->setValue(value);

        // Update the timer interval to match the new value.
        m_dataRefrTimer->setInterval( (int) ((((double)1/value)) * 1000));
    }
}

// Activates whenever the Start Data Collection Button is clicked.
// Resuming data collection causes strange bytes.
void RUNTEST::on_StartDCButton_clicked()
{
//    bool connectionOpened = false;
//    switch(mrtparent->collectionMethod())
//    {
//    case efidaq::COLLECTION_BY_SERIAL:
//        // Attempt to open the serial connection
//        connectionOpened = m_serialReader->open(QIODevice::ReadWrite);
//        break;
//    case efidaq::COLLECTION_BY_UDP:
//        // Attempt to bind to the current set address and port.
//        connectionOpened = m_udpReader->open();
//        break;
//    }

    if (m_serialReader->isOpen() && transceiver->startSendingData() == 0)
    //if (m_serialReader->isOpen())
    {
        // Starts the timer.
        if (!m_dataRefrTimer->isActive())
        {
           m_dataRefrTimer->start();
        }

        // Need to make the button become unclickable at this point
        ui->StartDCButton->setDisabled(true);

        // Needs to make sure the end data collection button is activated
        ui->EndDCButton->setDisabled(false);

        // Resizing the window while data is being collected can lag the system
        // dramatically. Until a better solution can be determined, this will
        // prevent the window from being resized while data is being collected.
        mrtparent->setResizeable(false);
    }
    else
    {
        notify(QString("Failed to open the connection."));
    }
}

// Activates whenever the End Data Collection Button is clicked.
// Ending data collection causes strange bytes.
void RUNTEST::on_EndDCButton_clicked()
{
    // Stops the timer
    if (m_dataRefrTimer->isActive())
    {
        m_dataRefrTimer->stop();
    }
    int stoppedSending = !transceiver->stopSendingData();
//    bool connectionClosed = false;
//    switch(mrtparent->collectionMethod())
//    {
//    case efidaq::COLLECTION_BY_SERIAL:
//        // Attempt to open the serial connection
//        connectionClosed = m_serialReader->close();
//        break;
//    case efidaq::COLLECTION_BY_UDP:
//        // Attempt to bind to the current set address and port.
//        connectionClosed = m_udpReader->close();
//        break;
//    }
    if (stoppedSending)
    //if (connectionClosed)
    {

        // Need to make this button become unactive at this point
        ui->EndDCButton->setDisabled(true);

        // Need to make the start data collection button become active. Also changes the text to "resume data collection."
        ui->StartDCButton->setDisabled(false);
        ui->StartDCButton->setText(QString("Resume Data Collection"));

        // Write any bytes waiting in the byte buffer to the screen
        //ui->DataBrowser->append(QString(*m_bytebuf));

        // Clear out the byte buffer
        m_bytebuf->clear();

        // Releases the lock on window resizing.
        mrtparent->setResizeable(true);
    }
    else
    {
        if (!m_dataRefrTimer->isActive())
        {
           m_dataRefrTimer->start();
        }
        notify("Failed to close the connection");
    }
}

void RUNTEST::handle_connectionTerminated()
{
    if (m_dataRefrTimer->isActive())
    {
        m_dataRefrTimer->stop();
    }

    // Need to make this button become unactive at this point
    ui->EndDCButton->setDisabled(true);

    // Need to make the start data collection button become active. Also changes the text to "resume data collection."
    ui->StartDCButton->setDisabled(false);
    ui->StartDCButton->setText(QString("Resume Data Collection"));

    // Write any bytes waiting in the byte buffer to the screen
    //ui->DataBrowser->append(QString(*m_bytebuf));

    // Clear out the byte buffer
    m_bytebuf->clear();

    // Releases the lock on window resizing.
    mrtparent->setResizeable(true);
}

// Activates whenever the Open AFR Table button is clicked.
void RUNTEST::on_OpenAFRTableButton_clicked()
{
    // This window is set to delete automatically.
    // Should change it so that this window is closed and deleted when
    // the RUNTEST window is closed and deleted.
    m_afrtable = new AFRTABLE(nullptr, transceiver);
    m_afrtable->show();
}

void RUNTEST::handleIPAddressEditFinished()
{
    if (!m_udpReader->setIPAddress(ui->IPEdit->text()))
    {
        notify("Invalid IP address.");
        ui->IPEdit->setText(m_udpReader->getIPAddressString());
    }
}

void RUNTEST::handlePortNoEditFinished()
{
    quint16 value = 0;
    value = ui->PortEdit->text().toUShort();
    if (!value)
    {
        notify("Invalid entry. Port number must be an unsigned short.");
        ui->PortEdit->setText(QString("%1").arg(m_udpReader->getPortNo()));
        return;
    }
    if (!m_udpReader->setPortNo(value))
    {
        notify("Invalid port number.");
        ui->PortEdit->setText(QString("%1").arg(m_udpReader->getPortNo()));
    }
}

void RUNTEST::handleStreamRefrTimeout()
{
    // Only autoscrolls if set to show new values.
    if (mrtparent->isAutoScrolling())
    {
        ui->DataBrowser->verticalScrollBar()->setValue(ui->DataBrowser->verticalScrollBar()->maximum());
    }
}

// Add Data to the plot
void RUNTEST::addData()
{
    for (int i = 0; i < pw.size(); i++)
    {
        pw[i]->addData(m_xData[i], m_yData[i]);
        m_xData[i].clear();
        m_yData[i].clear();
    }
}

// Activates whenever the Plot Data button is clicked.
void RUNTEST::on_PlotDataButton_clicked()
{
    m_xData.push_back(QVector<double>());
    m_yData.push_back(QVector<double>());
    pw.push_back(new PlotWindow(nullptr, this));
    pw[pw.size() - 1]->setWindowTitle(QString("Plot ID: %1").arg(pw.size() - 1));
    pw[pw.size() - 1]->setXLabel(m_xLabel);
    pw[pw.size() - 1]->setYLabel(m_yLabel);
    pw[pw.size() - 1]->show();
}

// Activates whenever the get serial port button has been clicked.
void RUNTEST::on_setSerialPortButton_clicked()
{
    if (ui->setSerialPortButton->text() == "Connected Serial Port:")
    {
        ui->serialPortNameLabel->setText(m_serialReader->selectPort());
        ui->setSerialPortButton->setText("Disconnect Serial Port");
    }
    else
    {
        m_serialReader->close();
        ui->setSerialPortButton->setText("Connected Serial Port:");
        ui->serialPortNameLabel->setText("None");
    }
}

// Activated whenever the xListView selection is changed.
void RUNTEST::xItemChanged(QModelIndex xindex)
{
    // Change string for x item.
    // Also save index of item.
    m_xlmodel->getItemAt(xindex.row(), m_xLabel.first);
    m_xLabel.second = xindex.row();
}

// Activated whenever the yListView selection is changed.
void RUNTEST::yItemChanged(QModelIndex yindex)
{
    // Change string for y item.
    // Also save the index of the item.
    m_ylmodel->getItemAt(yindex.row(), m_yLabel.first);
    m_yLabel.second = yindex.row();
}

// Returns true if currently collecting data.
bool RUNTEST::isCollectingData() const
{
    return m_dataRefrTimer->isActive();
}

Signals* RUNTEST::getTransceiver()
{
    return transceiver;
}


void RUNTEST::handle_triggerDataRefrTimer_button()
{

    if (m_dataRefrTimer->isActive())
    {
        m_dataRefrTimer->stop();
    }
    else
    {
        m_dataRefrTimer->start();
    }
}
