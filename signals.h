#ifndef SIGNALS_H
#define SIGNALS_H

#include <QTimer>
#include <QVector>

#define NUMBER_OF_AFR_ROWS 11
#define NUMBER_OF_AFR_COLUMNS 16
#define ROWS_TIMES_COLUMNS 160

#include "serialreader.h"

#define TIMEOUT_RETRANSMIT 100
#define TIMEOUT_EXIT 3000

class Signals
{
public:
    Signals(SERIALREADER* serialWriter);

    void reset();
    int startSendingData();
    int stopSendingData();
    void synchronizeParamters();
    int sendTable(const QVector<QVector<float>> &afr_table);
    QVector<float> receiveTable(const QVector<QVector<int>> &afr_requests);
    int setIdleFuelRatio(float value);
    int setCurrentFuelRatio(float value);
    int setResetFuelRatio(float value);
    int setDesiredRPM(int32_t value);
    int setDesiredO2(float value);

private:
    SERIALREADER* m_serialWriter;

};

#endif // SIGNALS_H
