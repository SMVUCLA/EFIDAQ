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
    QByteArray qBuffer(buffer);
    m_serialWriter->write(qBuffer);

    return;
}

int Signals::startSendingData()
{
    uint8_t buffer[1];
    buffer[0] = 1;
    QByteArray qBuffer(buffer);
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
        for(; i < data.length(); i++)
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
    QByteArray qBuffer(buffer);
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
        for(; i < data.length(); i++)
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

void Signals::sendTable(QList<QList<QString>> afr_table)
{
    union DataWrapper
    {
      char buffer[3+4*ROWS_TIMES_COLUMNS];
      struct Data
      {
          uint8_t ID;
          uint8_t numRows;
          uint8_t numCols;
          float tableValues[ROWS_TIMES_COLUMNS];
      } data;
    } dataWrapper;

    dataWrapper.data.ID = 4;
    dataWrapper.data.numRows = NUMBER_OF_AFR_ROWS;
    DataWrapper.data.numCols = NUMBER_OF_AFR_COLUMNS;
    for(int r = 0; r < NUMBER_OF_AFR_ROWS; r++)
    {
        for (int c = 0; c < NUMBER_OF_AFR_COLUMNS; c++)
        {
            dataWrapper.data.tableValues[r*NUMBER_OF_AFR_ROWS + c*NUMBER_OF_AFR_COLUMNS] = (afr_table[r][c]).toFloat();
        }
    }
    QByteArray qBuffer(dataWrapper.buffer);
    m_serialWriter->write(qBuffer);
    return;
}

QList<QList<QString>> Signals::receiveTable()
{
    uint8_t buffer[1];
    buffer[0] = 4;
    QByteArray qBuffer(buffer);
    m_serialWriter->write(qBuffer);

    union DataWrapper
    {
      char buffer[3+4*ROWS_TIMES_COLUMNS];
      struct Data
      {
          uint8_t ID;
          uint8_t numRows;
          uint8_t numCols;
          float tableValues[ROWS_TIMES_COLUMNS];
      } data;
    } dataWrapper;



    QByteArray data;
    unsigned long long nbytes = 0;
    while (nbytes == 0)
    {
        m_serialWriter->availableData(data)
    }

    dataWrapper.buffer = data.data();
    QList<QList<QString>> afr_table;

    for(int r = 0; r < NUMBER_OF_AFR_ROWS; r++)
    {
        for (int c = 0; c < NUMBER_OF_AFR_COLUMNS; c++)
        {
            afr_table[r][c] = QString::number(dataWrapper.data.tableValues[r*NUMBER_OF_AFR_ROWS + c*NUMBER_OF_AFR_COLUMNS]);
        }
    }
    delete(timer);
    return afr_table;
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
    QByteArray qBuffer(dataWrapper.buffer);
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
        for(; i < data.length(); i++)
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
    QByteArray qBuffer(dataWrapper.buffer);
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
        for(; i < data.length(); i++)
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
    QByteArray qBuffer(dataWrapper.buffer);
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
        for(; i < data.length(); i++)
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
    QByteArray qBuffer(dataWrapper.buffer);
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
        for(; i < data.length(); i++)
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
    QByteArray qBuffer(dataWrapper.buffer);
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
        for(; i < data.length(); i++)
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
