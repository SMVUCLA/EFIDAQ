#include "signals.h"
#include <QElapsedTimer>

Signals::Signals(SERIALREADER* serialWriter)
{
    m_serialWriter = serialWriter;
}

void Signals::reset()
{
    uint8_t buffer[1];
    buffer[0] = 0;
    QByteArray qBuffer(buffer,1);
    m_serialWriter->write(qBuffer);

    return;
}

int Signals::startSendingData()
{
    uint8_t buffer[1];
    buffer[0] = 1;
    QByteArray qBuffer(buffer,1);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 1 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

int Signals::stopSendingData()
{
    uint8_t buffer[1];
    buffer[0] = 2;
    QByteArray qBuffer(buffer,1);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 2 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

void Signals::synchronizeParamters()
{
    return;
}

int Signals::sendTable(const QVector<QVector<float>> &afr_table_values)
{
    union DataWrapper
    {
        char buffer[7];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            float value;
        } data;
    } dataWrapper;
    dataWrapper.data.ID = 4;
    QByteArray qbuffer;

    union Acknowledgemnent
    {
        char buffer[7];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            uint32_t backPadding;
        } data;
    } *ack;

    int cellsTimedOut = 0;
    QElapsedTimer timer;
    timer.start();
    QByteArray data;
    bool receivedFlag = flag;
    int i;
    int transmissions;
    char* dataAddress;

    for(int j = 0; j < afr_table_values.length(); j++)
    {
        dataWrapper.data.rowNum = afr_table_values[j][0];
        dataWrapper.data.colNum = afr_table_values[j][1];
        dataWrapper.data.value = afr_table_values[j][2];
        qbuffer.append(dataWrapper.buffer,7);
        m_serialWriter->write(qBuffer);

        transmissions = 1;
        receivedFlag = false;
        i = 0;
        timer.restart();

        while(true)
        {
            m_serialWriter->availableData(data);
            dataAddress = data.data();
            for(; i < data.length() - 7; i++)
            {
                ack = &(dataAddress[i]);
                if (ack->data.ID == dataWrapper.data.ID &&
                    ack->data.rowNum == dataWrapper.data.rowNum &&
                    ack->data.colNum == dataWrapper.data.colNum &&
                    ack->data.backPadding == 0x80000002)
                {
                    receivedFlag = true;
                    break;
                }
            }

            if(receivedFlag)
            {
                data.clear();
                break;
            }


            if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
            {
                transmission++;
                m_serialWriter->write(qBuffer);
            }
            if(timer.elapsed() > TIMEOUT_EXIT)
            {
                cellsTimedOut++;
                data.clear();
                break;
            }
        }
    }

    return cellsTimedOut;
}

QVector<float> Signals::receiveTable(const QVector<QVector<int>> &afr_requests)
{
    union DataWrapper
    {
        char buffer[3];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
        } data;
    } dataWrapper;
    dataWrapper.data.ID = 5;
    QByteArray qbuffer;

    union Acknowledgemnent
    {
        char buffer[11];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            float value;
            uint32_t backPadding;
        } data;
    } *ack;

    QVector<float> afr_values;
    QElapsedTimer timer;
    timer.start();
    QByteArray data;
    bool receivedFlag = flag;
    int i;
    int transmissions;
    char* dataAddress;

    for(int j = 0; j < afr_requests.length(); j++)
    {
        dataWrapper.data.rowNum = afr_requests[j][0];
        dataWrapper.data.colNum = afr_requests[j][1];
        qbuffer.append(dataWrapper.buffer,3);
        m_serialWriter->write(qBuffer);

        transmissions = 1;
        receivedFlag = false;
        i = 0;
        timer.restart();

        while(true)
        {
            m_serialWriter->availableData(data);
            dataAddress = data.data();
            for(; i <= data.length() - 11; i++)
            {
                ack = &(dataAddress[i]);
                if (ack->data.ID == dataWrapper.data.ID &&
                    ack->data.rowNum == dataWrapper.data.rowNum &&
                    ack->data.colNum == dataWrapper.data.colNum &&
                    ack->data.backPadding == 0x80000002)
                {
                    receivedFlag = true;
                    afr_values[j] = ack->data.value;
                    break;
                }
            }

            if(receivedFlag)
            {
                data.clear();
                break;
            }


            if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
            {
                transmission++;
                m_serialWriter->write(qBuffer);
            }
            if(timer.elapsed() > TIMEOUT_EXIT)
            {
                afr_values[j] = -1;
                data.clear();
                break;
            }
        }
    }

    return afr_values;
}

int Signals::setIdleFuelRatio(float value)
{
    union DataWrapper
    {
        char buffer[5];
        struct Data
        {
            uint8_t ID;
            float IFR;
        } data;
    } dataWrapper;

    dataWrapper.data.ID = 6;
    dataWrapper.data.IFR = value;
    QByteArray qBuffer(dataWrapper.buffer,5);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 6 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

int Signals::setCurrentFuelRatio(float value)
{
    union DataWrapper
    {
        char buffer[5];
        struct Data
        {
            uint8_t ID;
            float CFR;
        } data;
    } dataWrapper;

    dataWrapper.data.ID = 7;
    dataWrapper.data.CFR = value;
    QByteArray qBuffer(dataWrapper.buffer,5);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 7 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

int Signals::setResetFuelRatio(float value)
{
    union DataWrapper
    {
        char buffer[5];
        struct Data
        {
            uint8_t ID;
            float RFR;
        } data;
    } dataWrapper;

    dataWrapper.data.ID = 8;
    dataWrapper.data.RFR = value;
    QByteArray qBuffer(dataWrapper.buffer,5);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 8 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

int Signals::setDesiredRPM(int32_t value)
{
    union DataWrapper
    {
        char buffer[5];
        struct Data
        {
            uint8_t ID;
            int32_t RPM;
        } data;
    } dataWrapper;

    dataWrapper.data.ID = 9;
    dataWrapper.data.RPM = value;
    QByteArray qBuffer(dataWrapper.buffer,5);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 9 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}

int Signals::setDesiredO2(float value)
{
    union DataWrapper
    {
        char buffer[5];
        struct Data
        {
            uint8_t ID;
            float dO2;
        } data;
    } dataWrapper;

    dataWrapper.data.ID = 10;
    dataWrapper.data.dO2 = value;
    QByteArray qBuffer(dataWrapper.buffer,5);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

    QByteArray data;
    int i = 0;
    uint8_t* ID;
    uint32_t* backPadding;
    char* dataAddress;
    int transmissions = 1;

    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = &(dataAddress[i]);
            backPadding = &(dataAddress[i+1]);
            if (*ID == 7 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmission++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}
