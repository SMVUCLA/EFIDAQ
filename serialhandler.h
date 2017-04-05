#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QSerialPort>
#include <QObject>

const unsigned long DEFAULT_BAUD_RATE = 250000;

class QByteArray;

class SerialHandler : public QObject
{
    Q_OBJECT

public:
    SerialHandler();
    ~SerialHandler();
    bool open(QSerialPort::OpenMode openmode);
    bool isOpen() const;
    bool close();
    void flush();
    unsigned long long availableData(QByteArray& data);
    unsigned long long write(const QByteArray& data);
    bool waitForData(int msecs);
    QString selectPort();
    QString portName() const;
    void directRead(QByteArray &data);
private slots:
    void handle_ReadyRead_emitted();
    void handle_Error_emitted(QSerialPort::SerialPortError error);
private:
    QByteArray* data;
    QSerialPort* serialPort;
signals:
    void stopCollecting();
};

#endif // SERIALHANDLER_H
