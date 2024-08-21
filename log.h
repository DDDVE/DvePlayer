#ifndef LOG_H
#define LOG_H

#include <QDateTime>
#include <QString>


qint64 getMiliSecondTimeStamp();

#define LOG_FUNCTION_AND_LINE   "function=[" \
    << __PRETTY_FUNCTION__ << "], line=[" << __LINE__ << "], time="

#define LOG_ENTER_FUNCTION_AND_LINE   "[INFO] Enter. function=[" \
    << __PRETTY_FUNCTION__ << "], line=[" << __LINE__ << "], time="

#define LOG_LEAVE_FUNCTION_AND_LINE   "[INFO] Leave. function=[" \
    << __PRETTY_FUNCTION__ << "], line=[" << __LINE__ << "], time="




#endif // LOG_H
