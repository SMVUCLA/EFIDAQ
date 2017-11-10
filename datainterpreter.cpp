#include "datainterpreter.h"
#include <QString>
#include <float.h>

QString DataInterpreter::getString()
{
    return QString::asprintf("%d, %d, %f, %f, %f, %f, %f, %f, %d",
                                (*rd).bi.time,
                                (*rd).bi.totalRevolutions,
                                (*rd).bi.ECT,
                                (*rd).bi.IAT,
                                (*rd).bi.MAP,
                                (*rd).bi.TPS,
                                (*rd).bi.OIN,
                                (*rd).bi.totalPulseTime,
                                (*rd).bi.RPM
                            );
}

double DataInterpreter::getValue(int index)
{
    switch(index)
    {
    case 0:
        return (*rd).bi.time;
        break; //breaks are not needed, just in case
    case 1:
        return (*rd).bi.totalRevolutions;
        break;
    case 2:
        return (*rd).bi.ECT;
        break;
    case 3:
        return (*rd).bi.IAT;
        break;
    case 4:
        return (*rd).bi.MAP;
        break;
    case 5:
        return (*rd).bi.TPS;
        break;
    case 6:
        return (*rd).bi.OIN;
        break;
    case 7:
        return (*rd).bi.totalPulseTime;
        break;
    case 8:
        return (*rd).bi.RPM;
        break;
    default:
        return -FLT_MAX;
        break;
    }
}

void DataInterpreter::setBytes(void* rawBytesAddress)
{
    rd = (RawData*)rawBytesAddress;
    return;
}

void* DataInterpreter::getBytes()
{
    return rd;
}

int DataInterpreter::getNumBytes()
{
    return RAW_DATA_LENGTH;
}

int DataInterpreter::getNumFields()
{
    return RAW_DATA_LENGTH/4 - NUM_PAD_WORDS;
}

uint32_t DataInterpreter::getEnd()
{
    return (*rd).bi.end;
}

uint32_t DataInterpreter::getStart()
{
    return (*rd).bi.start;
}

////////////////////////////////

QString WriteInterpreter::getString()
{
    return QString::asprintf("%d, %d, %f, %f, %f, %f, %f, %f, %d",
                                (*rd).bi.time,
                                (*rd).bi.totalRevolutions,
                                (*rd).bi.ECT,
                                (*rd).bi.IAT,
                                (*rd).bi.MAP,
                                (*rd).bi.TPS,
                                (*rd).bi.OIN,
                                (*rd).bi.totalPulseTime,
                                (*rd).bi.RPM
                            );
}

double WriteInterpreter::getValue(int index)
{
    switch(index)
    {
    case 0:
        return (*rd).bi.time;
        break; //breaks are not needed, just in case
    case 1:
        return (*rd).bi.totalRevolutions;
        break;
    case 2:
        return (*rd).bi.ECT;
        break;
    case 3:
        return (*rd).bi.IAT;
        break;
    case 4:
        return (*rd).bi.MAP;
        break;
    case 5:
        return (*rd).bi.TPS;
        break;
    case 6:
        return (*rd).bi.OIN;
        break;
    case 7:
        return (*rd).bi.totalPulseTime;
        break;
    case 8:
        return (*rd).bi.RPM;
        break;
    default:
        return -FLT_MAX;
        break;
    }
}

void WriteInterpreter::setBytes(void* rawBytesAddress)
{
    rd = (RawData*)rawBytesAddress;
    return;
}

void* WriteInterpreter::getBytes()
{
    return rd;
}

int WriteInterpreter::getNumBytes()
{
    return DATA_LENGTH;
}

int WriteInterpreter::getNumFields()
{
    return DATA_LENGTH/4;
}
