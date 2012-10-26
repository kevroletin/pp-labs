#include <iostream>
#include <string>
#include <vector>
#include "pthread.h"
#include "semaphore.h"
#include <cassert>

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
// DEBUG: 
MixMasterLoger g_mutexLog(E_LOG_DEBUG);
MixMasterLoger g_semaphoreLog(E_LOG_DEBUG);
MixMasterLoger g_EventLog;
MixMasterLoger g_QueueLog;

/** Primitives */

class CMutex: protected MixSlaveLogger {
public:
    CMutex(std::string name, MixLogger& masterLog = g_mutexLog): 
        MixSlaveLogger(masterLog),
        m_name(name)
    {
        InitMutex(); 
    }
    ~CMutex() { pthread_mutex_destroy(&m_mutex); }
    int Get() {
        LOG_DEBUG(m_name + " -");
        return pthread_mutex_lock(&m_mutex);
    }
    int Put() {
        LOG_DEBUG(m_name + " +");
        return pthread_mutex_unlock(&m_mutex);
    }
    std::string GetName() { return m_name; }
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

class CSemaphore: protected MixSlaveLogger {
public:
    CSemaphore(std::string name, int init_value = 0, MixLogger& masterLog = g_semaphoreLog): 
        MixSlaveLogger(masterLog),
        m_name(name)
    {
        InitMutex(init_value); 
    }
    ~CSemaphore() { sem_destroy(&m_sem); } 
    int Get() {
        LOG_DEBUG(m_name + " -");
        return sem_wait(&m_sem);
    }
    int Put() {
        LOG_DEBUG(m_name + " +");
        return sem_post(&m_sem);
    }
    int GetValue() { 
        int buff = 0;
        assert( 0 == sem_getvalue(&m_sem, &buff) );
        return buff;
    }
    void SetValue(int val) {
        int sem_val = GetValue();
        for (int i = 0; i < sem_val - val; ++i) Get();
        for (int i = 0; i < val - sem_val; ++i) Put();
    }
    std::string GetName() { return m_name; }
protected:
    std::string m_name;
    sem_t m_sem;
    int InitMutex(int init_value) {
        return sem_init(&m_sem, 1, init_value);
    }
};

class CScopeMutex {
    
};

class CEvent {

};

enum EMessageType {
    MSG_CONTROL,
    MSG_APPLICATION
};

enum EControlCmd {
    CMD_RUN,
    CMD_SUSPEND,
    CMD_DIE
};

struct CMessage {
    virtual EMessageType GetType() = 0;
};

struct CControlMessage: CMessage {
    virtual EMessageType GetType() { return MSG_CONTROL; }
    EControlCmd command;
};

struct CApplicationMessage: CMessage {
    virtual EMessageType GetType() { return MSG_APPLICATION; }
};

class CSafeQueue {
public:
    CSafeQueue(std::string name): m_name(name), m_mutex(name + " protect mutex") {}
    
protected:
    std::string m_name;
    CMutex m_mutex;
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
    void RunLoop() {
        
    }
    virtual bool ProcessMessage() = 0;
};


int main(int argc, char* argv[])
{
    CSemaphore s("TestSemaphore");
    assert(0 == s.GetValue());
    s.Put();
    s.Put();
    assert(2 == s.GetValue());
    s.Get();
    assert(1 == s.GetValue());
    s.SetValue(4);
    assert(4 == s.GetValue());
    s.SetValue(0);
    assert(0 == s.GetValue());

    
    std::cerr << "Good\n";
    g_semaphoreLog.PublishLog(std::cerr);
    return 0;
}
