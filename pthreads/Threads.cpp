#include <iostream>
#include <string>
#include <vector>
#include "pthread.h"
#include "semaphore.h"
#include <cassert>
#include "Threads.h"

std::ostream& operator<<(std::ostream& out, CLogItem l) {
    out << l.m_timeMs << " - " << l.m_msg;
    return out;
};

TLogContainer& operator<<(TLogContainer& logContainer, CLogItem& logItem) {
    logContainer.push_back(logItem);
    return logContainer;
}

