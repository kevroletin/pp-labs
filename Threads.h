#ifndef _THREADS_H
#define _THREADS_H

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <deque>
#include "pthread.h"
#include "semaphore.h"

//#define THREADS_CERR_LOG

/** Primitives */

class CMutex {
public:
    CMutex(std::string name): 
        m_name(name)
    {
        InitMutex(); 
    }
    ~CMutex() { pthread_mutex_destroy(&m_mutex); }
    int Get() {
        return pthread_mutex_lock(&m_mutex);
    }
    int Put() {
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

class CScopeMutex {
public:
    CScopeMutex(CMutex& mutex): m_mutex(mutex) { m_mutex.Get(); }
    ~CScopeMutex() { m_mutex.Put(); }
protected:
    CMutex& m_mutex;
};

#define PROTECT CScopeMutex protect(m_mutex)

class CSemaphore {
public:
    CSemaphore(std::string name, int init_value = 0): 
        m_name(name)
    {
        InitMutex(init_value); 
    }
    ~CSemaphore() { sem_destroy(&m_sem); } 
    int Get() {
        return sem_wait(&m_sem);
    }
    int Put() {
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

/*
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
*/

struct CMessage {
CMessage(std::string str): m_type(0), m_tag(0), m_str(str) {}
    unsigned m_type;
    unsigned m_tag;
    std::string m_str;
    void* m_data;
};

class CSafeQueue {
public:
    CSafeQueue(std::string name):
        m_name(name),
        m_mutex(name + " queue protect mutex"),
        m_pop_sem(name + " queue pop semahore", 0) {}
    void Push(CMessage msg) {
	PROTECT;
	dataContainer.push_back(msg);
	m_pop_sem.Put();
    }
    CMessage Pop() {
	m_pop_sem.Get();
	{
	    PROTECT;
	    CMessage res = dataContainer.front();
	    dataContainer.pop_front();
	    return res;
	}
    }
    
protected:
    std::deque<CMessage> dataContainer;
    std::string m_name;
    CMutex m_mutex;
    CSemaphore m_pop_sem;
};

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

std::ostream& operator<<(std::ostream& out, CLogItem l);

class ILogger {
public:
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
    MixLogger(): m_mutex("log protect mutex") {}
    MixLogger(ELogLevel logLevel): m_logLevel(logLevel), m_mutex("log protect mutex") {}
    virtual ELogLevel GetLogLevel() { return m_logLevel; }
    ELogLevel m_logLevel;

    CMutex m_mutex;
};

#define LOG_ERROR(msg)   PutToLogCond(CLogItem(std::string("[error] ") + msg), E_LOG_ERRORS)
#define LOG_WARNING(msg) PutToLogCond(CLogItem(std::string("[warn ] ") + msg), E_LOG_WARNINGS)
#define LOG_INFO(msg)    PutToLogCond(CLogItem(std::string("[info ] ") + msg), E_LOG_INFO)
#define LOG_DEBUG(msg)   PutToLogCond(CLogItem(std::string("[debug] ") + msg), E_LOG_DEBUG)
#define LOG(msg)         PutToLogCond(CLogItem(msg), E_LOG_ERRORS)

typedef std::vector<CLogItem> TLogContainer;

TLogContainer& operator<<(TLogContainer& logContainer, CLogItem& logItem);

struct MixMasterLoger: MixLogger {
    MixMasterLoger(ELogLevel logLevel = E_LOG_DEBUG): MixLogger(logLevel) {}
    TLogContainer m_logContainer;
    virtual void PutToLog(CLogItem l) { 
	PROTECT;
	m_logContainer << l; 
    }
    virtual void PublishLog(std::ostream& out) {
	PROTECT;
        for (TLogContainer::iterator it = m_logContainer.begin(); it != m_logContainer.end(); ++it) {
            out << *it << "\n";
        }
    }
};

struct MixCerrLogger: MixLogger {
    MixCerrLogger(ELogLevel logLevel): MixLogger(logLevel) {}    
    virtual void PutToLog(CLogItem l) { 
	PROTECT;
	std::stringstream ss;
	ss << l << "\n";
	std::cerr << ss.str();
    }
};

struct MixSlaveLogger: ILogger {
    MixSlaveLogger(MixLogger& masterLog, ELogLevel logLevel = E_LOG_DEBUG):
        m_masterLog(masterLog) {}
    MixLogger& m_masterLog;
    virtual void PutToLog(CLogItem l) { m_masterLog.PutToLog(l); }
    virtual ELogLevel GetLogLevel() { return m_masterLog.GetLogLevel(); }
};

struct MixDummyLogger: ILogger {
    virtual void PutToLog(CLogItem l) { /* Dummy */ }
    virtual ELogLevel GetLogLevel() { return E_LOG_NONE; }
};

#ifdef THREADS_CERR_LOG
class CBasicThread: public MixCerrLogger {
#else
class CBasicThread: public MixMasterLoger {
#endif
public:
    CBasicThread(std::string name, ELogLevel logLevel = E_LOG_DEBUG):
#ifdef THREADS_CERR_LOG
        MixCerrLogger(logLevel),
#else
        MixMasterLoger(logLevel),
#endif	
        m_name(name)
    {
    }
    void Go()
    {
	pthread_create(&m_pthread, NULL, CBasicThread::StartThread, static_cast<void*>(this));
    }
    void* Join() {
        LOG_INFO(m_name + " join");
        void* buff;
        pthread_join(m_pthread, &buff);
        return buff;
    }
protected:
    void* Start() {
        LOG_INFO(m_name + " start");
        Body();
        return NULL;
    }
    static void* StartThread(void* param) {
        return static_cast<CBasicThread*>(param)->Start();
    }
    virtual void Body() = 0;

    std::string m_name;
    pthread_t m_pthread;
};

#endif
