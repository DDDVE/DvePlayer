#include "log.h"

qint64 getMiliSecondTimeStamp()
{
    return QDateTime::currentMSecsSinceEpoch();
}
