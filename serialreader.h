#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QSerialPort>
#include <QObject>
#include <QByteArray>
#include <QList>

class SERIALREADER : public QObject
{
    Q_OBJECT
public:
    SERIALREADER(QObject* parent = nullptr);
    ~SERIALREADER();
    bool open(QSerialPort::OpenMode openmode);
    bool isOpen() const;
    bool close();
    void flush();
    unsigned long long availableData(QByteArray& data);
    unsigned long long write(const QByteArray& data);
    QString selectPort();
    QString portName() const;

private:
    QByteArray* m_data;
    QSerialPort* m_serialPort;
private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
signals:
    void stopCollecting();
};

#endif // SERIALREADER_H
