#include "serialhandler.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QAbstractButton>

SerialHandler::SerialHandler()
{
    // Allocate memory
    data = new QByteArray;
    serialPort = new QSerialPort;
    serialPort->setPortName(QString("NONE"));
    serialPort->setBaudRate(DEFAULT_BAUD_RATE);

    // Connect signals to slots
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handle_Error_emitted(QSerialPort::SerialPortError)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(handle_ReadyRead_emitted()));
}
SerialHandler::~SerialHandler()
{
    delete data;
    delete serialPort;
}
// Attempts to open serial connection
bool SerialHandler::open(QSerialPort::OpenMode openmode)
{
    if (!serialPort->isOpen())
    {
        if (serialPort->open(openmode))
        {
            return true;
        }
    }
    return false;
}
// Attempts to close serial connection
bool SerialHandler::close()
{
    if (serialPort->isOpen())
    {
        serialPort->close();
        return true;
    }
    return false;
}

void SerialHandler::flush()
{
    handle_ReadyRead_emitted();
    data->clear();
}

// Gets the available data from the serial bytearray.
unsigned long long SerialHandler::availableData(QByteArray& data)
{
    // Holds the number of bytes returned
    unsigned long long nBytes = 0;

    // Append the current data to the input array
    data.append(*(this->data));

    nBytes = this->data->length();

    // Clear the byte array
    this->data->clear();

    return nBytes;
}

// Write data to the serial port.
unsigned long long SerialHandler::write(const QByteArray& data)
{
    if (serialPort->isOpen())
    {
        int n = serialPort->write(data);
        serialPort->waitForBytesWritten(-1);
        return n;
    }
    return 0 ;
}

bool SerialHandler::waitForData(int msecs)
{
    return serialPort->waitForReadyRead(msecs);
}

bool SerialHandler::isOpen() const
{
    return serialPort->isOpen();
}

// Attempts to select a serial port to use.
QString SerialHandler::selectPort()
{
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();

    QMessageBox determinePort;
    switch (availablePorts.length())
    {
    case 0:
        determinePort.setText(QString("No serial ports currently available."));
        determinePort.exec();
        return QString("NONE");
    case 1:
        serialPort->setPort(availablePorts[0]);
        open(QIODevice::ReadWrite);
        break;
    default:
        // Make message box to ask for which serial port to use.
        determinePort.setText(QString("Select the serial port to read from:"));
        determinePort.addButton(QString("NONE"), QMessageBox::RejectRole);
        for (int i = 0; i < availablePorts.length(); i++)
        {
            determinePort.addButton(QString("%1").arg(availablePorts[i].portName()), QMessageBox::ActionRole);
        }
        determinePort.exec();
        for (int i = 0; i < availablePorts.length(); i++)
        {
            if (determinePort.clickedButton()->text() == availablePorts[i].portName())
            {
                serialPort->setPort(availablePorts[i]);
                break;
            }
        }
        if (determinePort.clickedButton()->text() == "NONE")
        {
            return QString("NONE");
        }
        break;
    }
    return serialPort->portName();
}

// Returns the name of the currently selected port.
QString SerialHandler::portName() const
{
    return serialPort->portName();
}

// Transfers available data into the data read buffer
void SerialHandler::handle_ReadyRead_emitted()
{
    // Append the available data to the end of the byte array.
    data->append(serialPort->readAll());
}

void SerialHandler::directRead(QByteArray &data)
{
    data.append(serialPort->readAll());
}

// Handles errors
void SerialHandler::handle_Error_emitted(QSerialPort::SerialPortError error)
{
    QMessageBox errorMSGBOX;
    QString errorMSG = "";
    switch (error)
    {
    case QSerialPort::NoError:
        return;
    case QSerialPort::DeviceNotFoundError:
        errorMSG = "ERROR. No serial device found.";
        break;
    case QSerialPort::PermissionError:
        errorMSG = "PermissionError";
        break;
    case QSerialPort::OpenError:
        errorMSG = "OpenError";
        break;
    case QSerialPort::NotOpenError:
        errorMSG = "NotOpenError";
        break;
    case QSerialPort::WriteError:
        errorMSG = "WriteError";
        break;
    case QSerialPort::ReadError:
        errorMSG = "ReadError";
        break;
    case QSerialPort::ResourceError:
        errorMSG = "ERROR. The serial connection was abruptly terminated.";
        emit stopCollecting();
        break;

    case QSerialPort::UnsupportedOperationError:
        errorMSG = "UnsupportedOperationError";
        break;
    case QSerialPort::TimeoutError:
        return;
        errorMSG = "TimeoutError";
        break;
    default:
        break;
    }
    errorMSGBOX.setText(errorMSG);
    errorMSGBOX.exec();

    //QSerialPort::NoError                    0	No error occurred.
    //QSerialPort::DeviceNotFoundError        1	An error occurred while attempting to open an non-existing device.
    //QSerialPort::PermissionError            2	An error occurred while attempting to open an already opened device by another process or a user not having enough permission and credentials to open.
    //QSerialPort::OpenError                  3	An error occurred while attempting to open an already opened device in this object.
    //QSerialPort::NotOpenError               13	This error occurs when an operation is executed that can only be successfully performed if the device is open. This value was introduced in QtSerialPort 5.2.
    //QSerialPort::WriteError                 7	An I/O error occurred while writing the data.
    //QSerialPort::ReadError                  8  	An I/O error occurred while reading the data.
    //QSerialPort::ResourceError              9	An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.
    //QSerialPort::UnsupportedOperationError	10	The requested device operation is not supported or prohibited by the running operating system.
    //QSerialPort::TimeoutError               12	A timeout error occurred. This value was introduced in QtSerialPort 5.2.
    //QSerialPort::UnknownError
}

