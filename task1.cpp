#include <iostream>
#include <string>
#include <vector>
#include "pthread.h"

/** Logs */

enum ELogLevel {
    E_LOG_NONE,
    E_LOG_ERRORS,
    E_LOG_WARNINGS,
    E_LOG_INFO,
    E_LOG_DEBUG,
};

struct CLogItem {
    std::string m_msg;
    time_t m_timeMs;
    CLogItem(std::string msg): m_msg(msg) { SetCurrentTime(); }
    void SetCurrentTime() { m_timeMs = clock()*1000 / CLOCKS_PER_SEC; }
};

std::ostream& operator<<(std::ostream& out, CLogItem l) {
    out << l.m_timeMs << " - " << l.m_msg;
    return out;
};

struct ILogger {
    virtual ELogLevel GetLogLevel() = 0;
    virtual void PutToLog(CLogItem l) = 0;
    void PutToLogCond(CLogItem l, ELogLevel level) {
        if (level <= GetLogLevel()) {
            PutToLog(l);
        }
    }
    virtual void PublishLog(std::ostream& out) { out << "Log publishind not implemented\n"; }
};

struct MixLogger: ILogger {
    MixLogger(ELogLevel logLevel): m_logLevel(logLevel) {}
    virtual ELogLevel GetLogLevel() { return m_logLevel; }
    ELogLevel m_logLevel;
};

#define LOG_ERROR(msg)   PutToLogCond(CLogItem(std::string("[error] ") + msg), E_LOG_ERRORS)
#define LOG_WARNING(msg) PutToLogCond(CLogItem(std::string("[warn ] ") + msg), E_LOG_WARNINGS)
#define LOG_INFO(msg)    PutToLogCond(CLogItem(std::string("[info ] ") + msg), E_LOG_INFO)
#define LOG_DEBUG(msg)   PutToLogCond(CLogItem(std::string("[debug] ") + msg), E_LOG_DEBUG)
#define LOG(msg)         PutToLogCond(CLogItem(msg), E_LOG_ERRORS)

typedef std::vector<CLogItem> TLogContainer;

TLogContainer& operator<<(TLogContainer& logContainer, CLogItem& logItem) {
    logContainer.push_back(logItem);
    return logContainer;
}

struct MixMasterLoger: MixLogger {
    MixMasterLoger(ELogLevel logLevel = E_LOG_DEBUG): MixLogger(logLevel) {}
    TLogContainer m_logContainer;
    virtual void PutToLog(CLogItem l) { m_logContainer << l; }
    virtual void PublishLog(std::ostream& out) {
        for (TLogContainer::iterator it = m_logContainer.begin(); it != m_logContainer.end(); ++it) {
            out << *it << "\n";
        }
    }
};

struct MixCerrLogger: MixLogger {
    MixCerrLogger(ELogLevel logLevel): MixLogger(logLevel) {}    
    virtual void PutToLog(CLogItem l) { std::cerr << l; }
};

struct MixSlaveLogger: ILogger {
    MixSlaveLogger(MixLogger& masterLog, ELogLevel logLevel = E_LOG_DEBUG):
        m_masterLog(masterLog) {}
    MixLogger& m_masterLog;
    virtual void PutToLog(CLogItem l) { m_masterLog.PutToLog(l); }
    virtual ELogLevel GetLogLevel() { return m_masterLog.GetLogLevel(); }
};

struct MixDummyLogger: MixLogger {
    virtual void PutToLog(CLogItem l) { /* Dummy */ }    
};

/** Default Logs */

MixMasterLoger g_mutexLog(E_LOG_DEBUG);
//MixMasterLoger g_EventLog;
//MixMasterLoger g_QueueLog;

/** Privitives */

class CMutex: protected MixSlaveLogger {
public:
    CMutex(std::string name, MixLogger& masterLog = g_mutexLog): 
        MixSlaveLogger(masterLog),
        m_name(name)
    {
        InitMutex(); 
    }
    int Get() {
        LOG_DEBUG(m_name + " -");
        return pthread_mutex_lock(&m_mutex);
    }
    int Put() {
        LOG_DEBUG(m_name + " +");
        return pthread_mutex_unlock(&m_mutex);
    }
protected:
    std::string m_name;
    pthread_mutex_t m_mutex;
    int InitMutex() {
        pthread_mutexattr_t a;
        return
            pthread_mutexattr_init(&a) ||
            pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE) ||
            pthread_mutex_init(&m_mutex, &a) ||
            pthread_mutexattr_destroy(&a);
    }
};

class CScopeMutex {
    
};

class CEvent {

};

class IThreadQueue {
    
};

/** IThread
 *   executes infinute loop in separeta thread. Main loop calls
 *   ProcessMessage() on each message.
 */

class IThread {
public:
    void Start();
    void Suspend();
    void Join();
protected:
    std::string name;
    void RunLoop();
    virtual bool ProcessMessage() = 0;
};


int main(int argc, char* argv[])
{
    CMutex m("TestMutex");
    m.Get();
    m.Get();
    m.Put();
    m.Put();
    g_mutexLog.PublishLog(std::cerr);
    std::cerr << "Good";
    return 0;
}
