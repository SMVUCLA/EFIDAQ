#include "serialreader.h"
#include "utilities.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QList>
#include <QPushButton>

SERIALREADER::SERIALREADER(QObject* parent)
    :QObject(parent)
{
    // Allocate memory
    m_data = new QByteArray;
    m_serialPort = new QSerialPort;
    m_serialPort->setPortName(QString("NONE"));
    m_serialPort->setBaudRate(efidaq::DEFAULT_BAUD_RATE);
    //QSerialPort::Baud9600  <-- Maximum data integrity ~47 data points per second
    //QSerialPort::Baud19200 <-- Strong data integrity  ~94 data points per second
    //QSerialPort::Baud38400 <-- Good data integrity    ~141 data points per second
    //QSerialPort::Baud57600 <-- Okay data integrity    ~188 data points per second
    //QSerialPort::Baud115200<-- Marginal data integrity ~370 data points per second
    //Baud rate = 230400 <-- Bad data integrity     ~490 data points per second

    // Connect signals to slots
    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));

    // Uncomment this to let the user set the serial port when the window opens.
    this->selectPort();

}

SERIALREADER::~SERIALREADER()
{
    // Free memory
    delete m_data;
    delete m_serialPort;
}

// Attempts to open serial connection
bool SERIALREADER::open(QSerialPort::OpenMode openmode)
{
    if (!m_serialPort->isOpen())
    {
        if (m_serialPort->open(openmode))
        {
            connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
            return true;
        }
    }
    return false;
}

// Attempts to close serial connection
bool SERIALREADER::close()
{
    if (m_serialPort->isOpen())
    {
        disconnect(m_serialPort, SIGNAL(readyRead()));
        m_serialPort->close();
        return true;
    }
    return false;
}

void SERIALREADER::flush()
{
    handleReadyRead();
    m_data->clear();
}

// Gets the available data from the serial bytearray.
unsigned long long SERIALREADER::availableData(QByteArray& data)
{
    // Holds the number of bytes returned
    unsigned long long nBytes = 0;

    // Append the current data to the input array
    data.append(*m_data);

    nBytes = m_data->length();

    // Clear the byte array
    m_data->clear();

    return nBytes;
}

// Write data to the serial port.
unsigned long long SERIALREADER::write(const QByteArray& data)
{
    if (m_serialPort->isOpen())
    {
        return m_serialPort->write(data);
    }
    return 0 ;
}

// Attempts to select a serial port to use.
QString SERIALREADER::selectPort()
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
        m_serialPort->setPort(availablePorts[0]);
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
                m_serialPort->setPort(availablePorts[i]);
                break;
            }
        }
        if (determinePort.clickedButton()->text() == "NONE")
        {
            return QString("NONE");
        }
        break;
    }
    return m_serialPort->portName();
}

// Returns the name of the currently selected port.
QString SERIALREADER::portName() const
{
    return m_serialPort->portName();
}

// Transfers available data into the data read buffer
void SERIALREADER::handleReadyRead()
{
    // Append the available data to the end of the byte array.
    m_data->append(m_serialPort->readAll());
}

// Handles errors
void SERIALREADER::handleError(QSerialPort::SerialPortError error)
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


