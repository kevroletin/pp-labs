#include <iostream>
#include <string>
#include <vector>
#include "pthread.h"
#include "semaphore.h"
#include <cassert>
#include "Threads.h"

//MixMasterLoger g_mutexLog(E_LOG_DEBUG);
//MixCerrLogger g_mutexLog(E_LOG_DEBUG);
MixDummyLogger g_mutexLog;
MixMasterLoger g_semaphoreLog(E_LOG_DEBUG);
MixMasterLoger g_EventLog;
MixMasterLoger g_QueueLog;

std::ostream& operator<<(std::ostream& out, CLogItem l) {
    out << l.m_timeMs << " - " << l.m_msg;
    return out;
};

TLogContainer& operator<<(TLogContainer& logContainer, CLogItem& logItem) {
    logContainer.push_back(logItem);
    return logContainer;
}

