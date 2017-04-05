#include "daqwindow.h"
#include "ui_daqwindow.h"
#include "datainterpreter.h"
#include "plotwindow.h"
#include "listmodel.h"
#include "serialhandler.h"
#include "signals.h"

#include <utilities.h>
#include <QVector>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QAbstractButton>

DAQWindow::DAQWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DAQWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DEFAULT_LOGO_FILEPATH));

    // Connect Push button clicked handlers
    connect(ui->pushButton_Plot_Selections, SIGNAL(clicked(bool)), SLOT(handle_pushButton_Plot_Selections_clicked()));
    connect(ui->pushButton_Connect_To_USB_Port, SIGNAL(clicked(bool)), SLOT(handle_pushButton_Connect_To_USB_Port_clicked()));
    connect(ui->pushButton_Toggle_Collection, SIGNAL(clicked(bool)), SLOT(handle_pushButton_Toggle_Collection_clicked()));

    // Connect menu action handlers
    connect(ui->actionSave_As, SIGNAL(triggered(bool)), SLOT(handle_actionSave_As()));
    connect(ui->actionClear, SIGNAL(triggered(bool)), SLOT(handle_actionClear()));
    connect(ui->actionAutoscroll, SIGNAL(triggered(bool)), SLOT(handle_actionAutoscroll()));
    connect(ui->actionDisplay_Values, SIGNAL(triggered(bool)), SLOT(handle_actionDisplay_Values()));
    connect(ui->actionCommand_Window, SIGNAL(triggered(bool)), SLOT(handle_actionCommand_Window()));
    connect(ui->actionPlotting_Defaults, SIGNAL(triggered(bool)), SLOT(handle_actionPlotting_Defaults()));
    connect(ui->actionCollection_Defaults, SIGNAL(triggered(bool)), SLOT(handle_actionCollection_Defaults()));
    connect(ui->actionRefresh_Rate, SIGNAL(triggered(bool)), SLOT(handle_actionRefresh_Rate()));
    connect(ui->actionPlot_Anti_aliasing, SIGNAL(triggered(bool)), SLOT(handle_actionPlot_Anti_aliasing()));
    connect(ui->actionAFR_Feedback_K_d, SIGNAL(triggered(bool)), SLOT(handle_actionAFR_Feedback_K_d()));
    connect(ui->actionAFR_Feedback_K_p, SIGNAL(triggered(bool)), SLOT(handle_actionAFR_Feedback_K_p()));
    connect(ui->actionRPM_Feedback_K_p, SIGNAL(triggered(bool)), SLOT(handle_actionRPM_Feedback_K_p()));
    connect(ui->actionRPM_Feedback_K_d, SIGNAL(triggered(bool)), SLOT(handle_actionRPM_Feedback_K_d()));
    connect(ui->actionAFR_Table, SIGNAL(triggered(bool)), SLOT(handle_actionAFR_Table()));
    connect(ui->actionCold_Start_Adjustment, SIGNAL(triggered(bool)), SLOT(handle_actionCold_Start_Adjustment()));
    connect(ui->actionCollection_Window_Autoscroll, SIGNAL(triggered(bool)), SLOT(handle_actionCollection_Window_Autoscroll()));
    connect(ui->actionCollection_Window_Display_Values, SIGNAL(triggered(bool)), SLOT(handle_actionCollection_Window_Display_Values()));
    connect(ui->actionPlotting_Anti_aliasing, SIGNAL(triggered(bool)), SLOT(handle_actionPlotting_Anti_aliasing()));
    connect(ui->actionPlotting_Frame_Rate, SIGNAL(triggered(bool)), SLOT(handle_actionPlotting_Frame_Rate()));
    connect(ui->actionCollection_Window_Frame_Rate, SIGNAL(triggered(bool)), SLOT(handle_actionCollection_Window_Frame_Rate()));
    connect(ui->actionPredefined_Commands_Reset, SIGNAL(triggered(bool)), SLOT(handle_actionPredefined_Commands_Reset()));
    connect(ui->actionStart_Data_Transmission, SIGNAL(triggered(bool)), SLOT(handle_actionStart_Data_Transmission()));
    connect(ui->actionEnd_Data_Transmission, SIGNAL(triggered(bool)), SLOT(handle_actionEnd_Data_Transmission()));
    connect(ui->actionCollection_Window_Syntax_Highlighting, SIGNAL(triggered(bool)), SLOT(handle_actionCollection_Window_Syntax_Highlighting()));

    // Initialize member variables
    current_Collection_Refresh_Time = DEFAULT_COLLECTION_REFRESH_TIME;
    current_Collection_Window_Frame_Rate = DEFAULT_COLLECTION_WINDOW_FRAME_RATE;
    numDataPoints = 0;

    // Initialize Serial Handler
    serialHandler = new SerialHandler;
    connect(serialHandler, SIGNAL(stopCollecting()), SLOT(handle_connectionTerminated()));

    // Initialize transceiver
    transceiver = new Signals(serialHandler);

    // Initialize AFRTABLE
    afrTable = nullptr;

    // Initialize Label Lists
    xLabelModel.loadCSV(DEFAULT_XLABEL_LIST_FILENAME);
    yLabelModel.loadCSV(DEFAULT_YLABEL_LIST_FILENAME);
    ui->listView_x_labels->setModel(&xLabelModel);
    ui->listView_y_labels->setModel(&yLabelModel);
    connect(ui->listView_x_labels, SIGNAL(clicked(QModelIndex)), SLOT(xItemChanged(QModelIndex)));
    connect(ui->listView_x_labels, SIGNAL(activated(QModelIndex)), SLOT(xItemChanged(QModelIndex)));
    connect(ui->listView_y_labels, SIGNAL(clicked(QModelIndex)), SLOT(yItemChanged(QModelIndex)));
    connect(ui->listView_y_labels, SIGNAL(activated(QModelIndex)), SLOT(yItemChanged(QModelIndex)));

    // Initialize Timers
    timer_checkForInput.setInterval(DEFAULT_COLLECTION_REFRESH_TIME);
    connect(&timer_checkForInput, SIGNAL(timeout()), SLOT(handle_timer_checkForInput_timeout()));
    timer_refreshCollectionWindow.setInterval(1000/DEFAULT_COLLECTION_WINDOW_FRAME_RATE);
    connect(&timer_refreshCollectionWindow, SIGNAL(timeout()), SLOT(handle_timer_refreshCollectionWindow_timeout()));
    timer_refreshCollectionWindow.start(1000/DEFAULT_COLLECTION_WINDOW_FRAME_RATE);

    // Try to connect to a serial port
    handle_pushButton_Connect_To_USB_Port_clicked();

    if (ui->actionCollection_Window_Syntax_Highlighting->isChecked())
    {
        // Set up the syntax highlighter if default setting put it on.
        syntaxHighlighter = new Highlighter(ui->textEdit_Collection_Window->document());
    }
}

DAQWindow::~DAQWindow()
{
    delete ui;
    for (auto it = pw.begin(); it != pw.end();)
    {
        (*it)->hide();
        delete *it;
        it = pw.erase(it);
    }
    delete transceiver;
    if (afrTable != nullptr)
    {
        afrTable->hide();
        delete afrTable;
        afrTable = nullptr;
    }
    delete serialHandler;
}
// Serial handlers
void DAQWindow::handle_connectionTerminated()
{
    if (timer_checkForInput.isActive())
    {
        timer_checkForInput.stop();
    }
    // Toggle the button to be able to start collection again.
    ui->pushButton_Toggle_Collection->setText("Resume Data Collection");

    // Reset serial handler
    serialHandler->close();
    ui->pushButton_Connect_To_USB_Port->setText("Connect to USB Port:");
    ui->label_Connected_Port->setText("Connected Port: NONE");

    // Clear out the byte buffer
    carryoverByteBuffer.clear();

    // Releases the lock on window resizing.
    this->setResizeable(true);
}

// Timer timeout handlers
void DAQWindow::handle_timer_checkForInput_timeout()
{
    //ui->statusbar->showMessage(QString("%1").arg(numDataPoints));
    QByteArray data(carryoverByteBuffer);
    unsigned long long nBytes = 0;

    serialHandler->availableData(data);

    carryoverByteBuffer.clear();

    // Check to see if any bytes were recorded
    if (nBytes <= 0 && data.isEmpty())
    {
        return;
    }

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

        collectionWindowBuffer.append(interpreter.getString() + '\n');
        ui->statusbar->showMessage(interpreter.getString());
        dataBuffer.append(&(((char*)interpreter.getBytes())[4]) , interpreter.getNumBytes()-8);

        // Appends the xData and yData points
        // Check is necessary to ensure index is within the range of data input.
        for (int i = 0; i < pw.size(); i++)
        {
            if (pw[i]->getXLabelIndex() < interpreter.getNumFields() && pw[i]->getYLabelIndex() < interpreter.getNumFields())
            {
                double xval = interpreter.getValue(pw[i]->getXLabelIndex());
                plot_xData[i].append(xval);

                double yval = interpreter.getValue(pw[i]->getYLabelIndex());
                plot_yData[i].append(yval);
            }
        }
        numDataPoints++;
    }

    if(offset == data.length()- numBytesPerMessage + 1)
    {
        carryoverByteBuffer.append(&(dataAddress[offset - 1]), data.length()-(offset-1));
    }
    else if (data.length() - numBytesPerMessage + 1 < offset && offset < data.length())
    {
        carryoverByteBuffer.append(&(dataAddress[offset]), data.length() - offset);
    }
    else
    {
        //This should only happen if data.length() = offset, in which case, there shoul be nothing to do, else there
        //is an error.
    }

    // Update plots
    updatePlotData();
}
void DAQWindow::handle_timer_refreshCollectionWindow_timeout()
{
    if (ui->actionCollection_Window_Display_Values->isChecked())
    {
        ui->textEdit_Collection_Window->insertPlainText(collectionWindowBuffer);
    }
    if (ui->actionCollection_Window_Autoscroll->isChecked())
    {
        ui->textEdit_Collection_Window->verticalScrollBar()->setValue(ui->textEdit_Collection_Window->verticalScrollBar()->maximum());
    }
    ui->lcdNumber_numDataPoints->display(numDataPoints);
    collectionWindowBuffer.clear();
}

// Other signal handlers
void DAQWindow::handle_AFRTableClosing_emitted()
{
    disconnect(afrTable, SIGNAL(closing()));
    delete afrTable;
    afrTable = nullptr;
}

// Functions for controlling plots.
void DAQWindow::xItemChanged(QModelIndex xindex)
{
    // Change string for x item.
    // Also save index of item.
    current_xLabel = xLabelModel.getItemAt(xindex.row());
    current_xLabelIndex = xindex.row();
}
void DAQWindow::yItemChanged(QModelIndex yindex)
{
    // Change string for y item.
    // Also save the index of the item.
    current_yLabel = yLabelModel.getItemAt(yindex.row());
    current_yLabelIndex = yindex.row();
}
void DAQWindow::updatePlotData()
{
    for (int i = 0; i < pw.size(); i++)
    {
        pw[i]->addData(plot_xData[i], plot_yData[i]);
        plot_xData[i].clear(); plot_yData[i].clear();
    }
}
void DAQWindow::stopPlotting(PlotWindow* sender)
{
    for (auto it = pw.begin(); it != pw.end();)
    {
        if (*it == sender)
        {
            delete *it;
            it = pw.erase(it);
            plot_xData.pop_back();
            plot_yData.pop_back();
        }
        else
        {
             it++;
        }
    }
}

// Push button clicked handlers
void DAQWindow::handle_pushButton_Plot_Selections_clicked()
{
    plot_xData.push_back(QVector<double>());
    plot_yData.push_back(QVector<double>());
    PlotWindow* newPlot = new PlotWindow(nullptr, this);
    newPlot->setWindowTitle(QString("Plot of %1 vs. %2").arg(current_yLabel).arg(current_xLabel));
    newPlot->setXLabel(current_xLabel, current_xLabelIndex);
    newPlot->setYLabel(current_yLabel, current_yLabelIndex);
    pw.push_back(newPlot);
    newPlot->show();
}
void DAQWindow::handle_pushButton_Connect_To_USB_Port_clicked()
{
    if (ui->pushButton_Connect_To_USB_Port->text() == "Connect to USB Port:")
    {
        QString selectedPort = serialHandler->selectPort();
        if (selectedPort == "NONE")
        {
            return;
        }
        ui->label_Connected_Port->setText(QString("Connected Port: ") + serialHandler->selectPort());
        ui->pushButton_Connect_To_USB_Port->setText("Disconnect from USB Port:");
    }
    else
    {
        serialHandler->close();
        ui->pushButton_Connect_To_USB_Port->setText("Connect to USB Port:");
        ui->label_Connected_Port->setText("Connected Port: NONE");
    }
}
void DAQWindow::handle_pushButton_Toggle_Collection_clicked()
{
    if (ui->pushButton_Toggle_Collection->text() == "Start Data Collection" ||
            ui->pushButton_Toggle_Collection->text() == "Resume Data Collection")
    {
        if (serialHandler->isOpen() && transceiver->startSendingData() == 0)
        {
            // Starts the timer.
            if (!timer_checkForInput.isActive())
            {
               timer_checkForInput.start();
            }

            // Toggle the button to be able to stop data collection.
            ui->pushButton_Toggle_Collection->setText("Stop Data Collection");

            // Resizing the window while data is being collected can lag the system
            // dramatically. Until a better solution can be determined, this will
            // prevent the window from being resized while data is being collected.
            this->setResizeable(false);
        }
        else
        {
            QMessageBox::warning(this, "ERROR",
                                 "Failed to open the connection."
                                 );
        }
    }
    else if (ui->pushButton_Toggle_Collection->text() == "Stop Data Collection")
    {
        // Stops the timer
        if (timer_checkForInput.isActive())
        {
            timer_checkForInput.stop();
        }

        // Sends a signal to the controller to stop sending data.
        int stoppedSending = !transceiver->stopSendingData();

        // Checks the result of the signal to the controller.
        if (stoppedSending)
        {
            // Toggle the button to be able to start collection again.
            ui->pushButton_Toggle_Collection->setText("Resume Data Collection");

            // Clear out the byte buffer
            carryoverByteBuffer.clear();

            // Releases the lock on window resizing.
            this->setResizeable(true);
        }
        else
        {
            if (!timer_checkForInput.isActive())
            {
               timer_checkForInput.start();
            }
            QMessageBox::warning(this, "ERROR",
                                 "Failed to close the connection."
                                 );
        }
    }
    else
    {
        QMessageBox::warning(this, "ERROR",
                             "Unexpected text appeared in the \"ui->pushButton_Toggle_Collection->text() output\". An unexpected case was triggered."
                             );
    }
}

// Menu action handlers
void DAQWindow::handle_actionSave_As()
{
    saveAs();
}
void DAQWindow::handle_actionClear()
{
    clear();
}
void DAQWindow::handle_actionAutoscroll()
{
    return;
}
void DAQWindow::handle_actionDisplay_Values()
{
    return;
}
void DAQWindow::handle_actionCommand_Window(){}
void DAQWindow::handle_actionPlotting_Defaults(){}
void DAQWindow::handle_actionCollection_Defaults(){}
void DAQWindow::handle_actionRefresh_Rate()
{
    bool ok;
    int input = QInputDialog::getInt(this,
                                     "Specify Maximum Collection Refresh Rate",
                                     "Refresh Time (ms/refresh)",
                                     current_Collection_Refresh_Time,
                                     MINIMUM_COLLECTION_REFRESH_TIME,
                                     MAXIMUM_COLLECTION_REFRESH_TIME,
                                     1,
                                     &ok);
    if (ok)
    {
        current_Collection_Refresh_Time = input;
        timer_checkForInput.setInterval(current_Collection_Refresh_Time);
    }
}
void DAQWindow::handle_actionPlot_Anti_aliasing()
{
    return;
}
void DAQWindow::handle_actionAFR_Feedback_K_d(){}
void DAQWindow::handle_actionAFR_Feedback_K_p(){}
void DAQWindow::handle_actionRPM_Feedback_K_p(){}
void DAQWindow::handle_actionRPM_Feedback_K_d(){}
void DAQWindow::handle_actionAFR_Table()
{
    if (afrTable == nullptr)
    {
        afrTable = new AFRTABLE(nullptr, transceiver);
        connect(afrTable, SIGNAL(closing()), SLOT(handle_AFRTableClosing_emitted()));
        afrTable->show();
    }
}
void DAQWindow::handle_actionCold_Start_Adjustment(){}
void DAQWindow::handle_actionCollection_Window_Autoscroll()
{
    return;
}
void DAQWindow::handle_actionCollection_Window_Display_Values()
{
    return;
}
void DAQWindow::handle_actionPlotting_Anti_aliasing(){}
void DAQWindow::handle_actionPlotting_Frame_Rate(){}
void DAQWindow::handle_actionCollection_Window_Frame_Rate()
{
    bool ok;
    int input = QInputDialog::getInt(this,
                                     "Specify Maximum Collection Frame Rate",
                                     "Frame Rate (frames/sec)",
                                     current_Collection_Window_Frame_Rate,
                                     MINIMUM_COLLECTION_WINDOW_FRAME_RATE,
                                     MAXIMUM_COLLECTION_WINDOW_FRAME_RATE,
                                     1,
                                     &ok);
    if (ok)
    {
        current_Collection_Window_Frame_Rate = input;
        timer_checkForInput.setInterval(1000/current_Collection_Window_Frame_Rate);
    }
}
void DAQWindow::handle_actionPredefined_Commands_Reset(){}
void DAQWindow::handle_actionStart_Data_Transmission()
{
    if (serialHandler->isOpen() && transceiver->startSendingData() == 0)
    {
        // Starts the timer.
        if (!timer_checkForInput.isActive())
        {
           timer_checkForInput.start();
        }

        // Toggle the button to be able to stop data collection.
        ui->pushButton_Toggle_Collection->setText("Stop Data Collection");

        // Resizing the window while data is being collected can lag the system
        // dramatically. Until a better solution can be determined, this will
        // prevent the window from being resized while data is being collected.
        this->setResizeable(false);
    }
    else
    {
        QMessageBox::warning(this, "ERROR",
                             "Failed to open the connection."
                             );
    }
}
void DAQWindow::handle_actionEnd_Data_Transmission()
{
    // Stops the timer
    if (timer_checkForInput.isActive())
    {
        timer_checkForInput.stop();
    }

    // Sends a signal to the controller to stop sending data.
    int stoppedSending = !transceiver->stopSendingData();

    // Checks the result of the signal to the controller.
    if (stoppedSending)
    {
        // Toggle the button to be able to start collection again.
        ui->pushButton_Toggle_Collection->setText("Resume Data Collection");

        // Clear out the byte buffer
        carryoverByteBuffer.clear();

        // Releases the lock on window resizing.
        this->setResizeable(true);
    }
    else
    {
        if (!timer_checkForInput.isActive())
        {
           timer_checkForInput.start();
        }
        QMessageBox::warning(this, "ERROR",
                             "Failed to close the connection."
                             );
    }
}
void DAQWindow::handle_actionCollection_Window_Syntax_Highlighting()
{
    if (ui->actionCollection_Window_Syntax_Highlighting->isChecked())
    {
        // Set up the syntax highlighter for the DataBrowser
        syntaxHighlighter = new Highlighter(ui->textEdit_Collection_Window->document());
    }
    else
    {
        delete syntaxHighlighter;
    }
}

// Events reimplemented
void DAQWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgbox;
    msgbox.setText("Would you like to save the current data before exiting?");
    QAbstractButton* a = msgbox.addButton(QMessageBox::Yes);
    QAbstractButton* b = msgbox.addButton(QMessageBox::No);
    msgbox.addButton(QMessageBox::Cancel);
    msgbox.exec();
    QAbstractButton* d = msgbox.clickedButton();
    if (timer_checkForInput.isActive())
    {
        handle_actionEnd_Data_Transmission();
    }
    if (d == a)
    {
        if (saveAs())
        {
            for (auto it = pw.begin(); it != pw.end();)
            {
                (*it)->hide();
                delete *it;
                it = pw.erase(it);
            }
            if (afrTable != nullptr)
            {
                afrTable->hide();
                delete afrTable;
                afrTable = nullptr;
            }
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
                for (auto it = pw.begin(); it != pw.end();)
                {
                    (*it)->hide();
                    delete *it;
                    it = pw.erase(it);
                }
                if (afrTable != nullptr)
                {
                    afrTable->hide();
                    delete afrTable;
                    afrTable = nullptr;
                }
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
        for (auto it = pw.begin(); it != pw.end();)
        {
            (*it)->hide();
            delete *it;
            it = pw.erase(it);
        }
        if (afrTable != nullptr)
        {
            afrTable->hide();
            delete afrTable;
            afrTable = nullptr;
        }
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

// Other functions
void DAQWindow::setResizeable(bool resizeable)
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
bool DAQWindow::saveAs()
{
    if (timer_checkForInput.isActive())
    {
        QMessageBox::warning(this, "ERROR",
                             "Cannot save while data is being collected."
                             );
        return false;
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
            QMessageBox::information(this, "Notification",
                                 "Saving cancelled."
                                 );
            return false;
        }
        QFile file(filename, this);
        if (!file.open(QFileDevice::WriteOnly))
        {
            QMessageBox::warning(this, "ERROR",
                                 "Failed to open specified file for writing."
                                 );
            return false;
        }
        if (extension == "BitFile(*.bit)")
        {
            if (file.write(dataBuffer) == -1)
            {
                file.close();
                QMessageBox::warning(this, "ERROR",
                                     "Failed to write to specified file."
                                     );
                return false;
            }
        }
        else
        {
            WriteInterpreter writer;
            QString fileContents("");
            char* bufferAddress = dataBuffer.data();
            int i;
            for(i = 0; i < dataBuffer.length()/writer.getNumBytes(); i++)
            {
                writer.setBytes(&(bufferAddress[i*writer.getNumBytes()]));
                fileContents += writer.getString() + "\n";
            }
            QTextStream out(&file);
            out << fileContents;
        }
        file.close();
        QMessageBox::information(this, "Notification",
                             "The data was successfully saved."
                             );
        return true;
    }
}
bool DAQWindow::clear()
{
    if (timer_checkForInput.isActive())
    {
        QMessageBox::warning(this, "ERROR",
                             "Cannot clear data while data is being collected."
                             );
        return false;
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
            ui->textEdit_Collection_Window->clear();
            dataBuffer.clear();
            numDataPoints = 0;
            ui->pushButton_Toggle_Collection->setText("Start Data Collection");
            QMessageBox::information(this, "Notification",
                                 "The data was successfully cleared."
                                 );
            return true;
        }
        else
        {
            QMessageBox::information(this, "Notification",
                                 "Clearing cancelled."
                                 );
            return false;
        }
    }
}
