//#define DEBUG
#include "signals.h"
#include <QElapsedTimer>

#ifdef DEBUG
    #include <QDebug>
#endif

Signals::Signals(SerialHandler* serialWriter)
{
    m_serialWriter = serialWriter;
}

void Signals::reset()
{
    uint8_t buffer[3];
    buffer[0] = 0;
    uint16_t checkSum = checkSumInitial;
   ((uint16_t *)(&buffer[1]))[0] = checkSum;
    QByteArray qBuffer((char*)buffer,3);
    m_serialWriter->write(qBuffer);

    return;
}

int Signals::startSendingData()
{
    QByteArray data;
    m_serialWriter->availableData(data);
    data.clear();

    uint8_t buffer[3];
    buffer[0] = 1;
    uint16_t checkSum = checkSumInitial-1;
    ((uint16_t *)(&buffer[1]))[0] = checkSum;
    QByteArray qBuffer((char*)buffer,3);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();
    int i = 0;
    uint8_t *ID;
    uint32_t *backPadding;
    char* dataAddress;
    int transmissions = 1;
    int ret;



    while(timer.elapsed() < TIMEOUT_EXIT)
    {
        ret = m_serialWriter->availableData(data);
        dataAddress = data.data();
        for(; i <= data.length() - 5; i++)
        {
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);

#ifdef DEBUG
            if (*ID == 1)
            {
                qDebug() << "ID =" << *ID;
                qDebug() << *backPadding;
                //return 0;
            }
#endif

            if (*ID == 1 && *backPadding == 0x80000002)
            {
#ifdef DEBUG
    qDebug() << "i = " << i;
#endif
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
            m_serialWriter->write(qBuffer);
        }
    }
#ifdef DEBUG
            qDebug() << i;
#endif
    return -1;

//    int i = 0;
//    union Acknowledgement
//    {
//        char buffer[8];
//        struct Data
//        {
//            uint8_t ID;
//            uint8_t alignmentPadding[3];
//            uint32_t backPadding;
//        } data;
//    } *ack;
//    char* dataAddress;
//    int transmissions = 1;
//    int ret;



//    while(timer.elapsed() < TIMEOUT_EXIT)
//    {
//        ret = m_serialWriter->availableData(data);
//        dataAddress = data.data();
//        for(; i <= data.length() - 8; i++)
//        {
//            ack = (Acknowledgement*)(&dataAddress[i]);

//#ifdef DEBUG
//            if (ack->data.ID == 1)
//            {
//                qDebug() << "ID =" << ack->data.ID;
//                qDebug() << ack->data.backPadding;
//                //return 0;
//            }
//#endif

//            if (ack->data.ID == 1 && ack->data.backPadding == 0x80000002)
//            {
//#ifdef DEBUG
//    qDebug() << "i = " << i;
//#endif
//                return 0;
//            }
//        }
//        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
//        {
//            transmissions++;
//            m_serialWriter->write(qBuffer);
//        }
//    }
//#ifdef DEBUG
//            qDebug() << i;
//#endif
//    return -1;
}

int Signals::stopSendingData()
{
    QByteArray data;
    m_serialWriter->availableData(data);
    data.clear();

    uint8_t buffer[3];
    buffer[0] = 2;
    uint16_t checkSum= checkSumInitial-2;
    ((uint16_t*)(&buffer[1]))[0]= checkSum;
    QByteArray qBuffer((char*)buffer,3);
    m_serialWriter->write(qBuffer);

    QElapsedTimer timer;
    timer.start();

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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);

#ifdef DEBUG
            if (*ID == 2)
            {
                qDebug() << "ID =" << *ID;
                qDebug() << *backPadding;
                //return 0;
            }
#endif

            if (*ID == 2 && *backPadding == 0x80000002)
            {
#ifdef DEBUG
    qDebug() << "i = " << i;
#endif
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
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
        char buffer[10];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            uint8_t alignmentPadding;
            float value;
            uint16_t checkSum;
        } data;
    } dataWrapper;
    dataWrapper.data.ID = 4;
    QByteArray qBuffer;

    union Acknowledgement
    {
        char buffer[8];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            uint8_t alignmentPadding;
            uint32_t backPadding;
        } data;
    } *ack;

    int cellsTimedOut = 0;
    QElapsedTimer timer;
    timer.start();
    QByteArray data;
    bool receivedFlag = false;
    int i;
    int transmissions;
    char* dataAddress;
    bool availableDataFlag;

    for(int j = 0; j < afr_table_values.length(); j++)
    {
        dataWrapper.data.rowNum = afr_table_values[j][0];
        dataWrapper.data.colNum = afr_table_values[j][1];
        dataWrapper.data.value = afr_table_values[j][2];
        unsigned char * valuePointer = reinterpret_cast<unsigned char *>(&(dataWrapper.data.value));
        dataWrapper.data.checkSum = checkSumInitial - (dataWrapper.data.ID)
                                                    - (dataWrapper.data.rowNum << 1)
                                                    - (dataWrapper.data.colNum << 2)
                                                    - ((uint16_t)(valuePointer[0]) << 3)
                                                    - ((uint16_t)(valuePointer[1]) << 4)
                                                    - ((uint16_t)(valuePointer[2]) << 5)
                                                    - ((uint16_t)(valuePointer[3]) << 6);
        qBuffer.clear();
        qBuffer.append(dataWrapper.buffer,10);

        m_serialWriter->availableData(data);
        data.clear();

        m_serialWriter->write(qBuffer);
        //availableDataFlag = m_serialWriter->waitForData(50);

        transmissions = 1;
        receivedFlag = false;
        i = 0;
        timer.restart();

        while(true)
        {
            m_serialWriter->availableData(data);
            dataAddress = data.data();
            for(; i <= data.length() - 8; i++)
            {
                ack = (Acknowledgement*) &(dataAddress[i]);
#ifdef DEBUG
                if(ack->data.ID == 4)
                {
                    qDebug()<<1;
                    if(ack->data.rowNum == dataWrapper.data.rowNum)
                    {
                        qDebug()<<2;
                        if(ack->data.colNum == dataWrapper.data.colNum)
                        {
                            qDebug()<<3;
                            qDebug()<<ack->data.backPadding;
                        }
                    }
                }
#endif
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
                transmissions++;
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
#ifdef DEBUG
    qDebug() << "i = " << i;
#endif
    return cellsTimedOut;
}

QVector<float> Signals::receiveTable(const QVector<QVector<int>> &afr_requests)
{
    union DataWrapper
    {
        char buffer[6];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            uint8_t alignmentPadding;
            uint16_t checkSum;
        } data;
    } dataWrapper;
    dataWrapper.data.ID = 5;
    QByteArray qBuffer;

    union Acknowledgement
    {
        char buffer[12];
        struct Data
        {
            uint8_t ID;
            uint8_t rowNum;
            uint8_t colNum;
            uint8_t alignmentPadding;
            float value;
            uint32_t backPadding;
        } data;
    } *ack;

    QVector<float> afr_values;
    QElapsedTimer timer;
    timer.start();
    QByteArray data;
    bool receivedFlag = false;
    int i;
    int transmissions;
    char* dataAddress;
    bool availableDataFlag;

    for(int j = 0; j < afr_requests.length(); j++)
    {
        #ifdef DEBUG
            qDebug() << "afr_request.length() = " << afr_requests.length();
        #endif
        dataWrapper.data.rowNum = afr_requests[j][0];
        dataWrapper.data.colNum = afr_requests[j][1];
        dataWrapper.data.checkSum = checkSumInitial - (dataWrapper.data.ID)
                                                    - (dataWrapper.data.rowNum << 1)
                                                    - (dataWrapper.data.colNum << 2);
        qBuffer.clear();
        qBuffer.append(dataWrapper.buffer,6);

        m_serialWriter->availableData(data);
        data.clear();

        m_serialWriter->write(qBuffer);
        availableDataFlag = m_serialWriter->waitForData(8);

        transmissions = 1;
        receivedFlag = false;
        i = 0;
        timer.restart();

        while(true)
        {
            m_serialWriter->availableData(data);
            dataAddress = data.data();
            for(; i <= data.length() - 12; i++)
            {
                #ifdef DEBUG
                    qDebug() << "here, Transmissions = " << transmissions << ", data length = " << data.length();
                #endif
                ack = (Acknowledgement*) &(dataAddress[i]);
                if (ack->data.ID == dataWrapper.data.ID &&
                    ack->data.rowNum == dataWrapper.data.rowNum &&
                    ack->data.colNum == dataWrapper.data.colNum &&
                    ack->data.backPadding == 0x80000002)
                {
                    receivedFlag = true;
                    afr_values.append(ack->data.value);
                    break;
                }
            }

            if(receivedFlag)
            {
                #ifdef DEBUG
                    qDebug() << "Total Transmission " << transmissions;
                #endif
                data.clear();
                break;
            }

            if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
            {
                #ifdef DEBUG
                    qDebug() << "data length = " << data.length();
                #endif
                transmissions++;
                m_serialWriter->write(qBuffer);

            }
            if(timer.elapsed() > TIMEOUT_EXIT)
            {
                afr_values.append(-1);
                data.clear();
                break;
            }
        }
    }
#ifdef DEBUG
    qDebug() << "i = " << i;
#endif
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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);
            if (*ID == 6 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);
            if (*ID == 7 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);
            if (*ID == 8 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);
            if (*ID == 9 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
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
            ID = (uint8_t*) &(dataAddress[i]);
            backPadding = (uint32_t*) &(dataAddress[i+1]);
            if (*ID == 7 && *backPadding == 0x80000002)
            {
                return 0;
            }
        }
        if(timer.elapsed() > transmissions*TIMEOUT_RETRANSMIT)
        {
            transmissions++;
            m_serialWriter->write(qBuffer);
        }
    }

    return -1;
}
