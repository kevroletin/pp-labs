#include <iostream>
#include <string>
#include "Threads.h"
#include <cassert>

enum EBankOperation {
    OP_ADD,
    OP_SUB,
    OP_LOCK,
    OP_UNLOCK,
    OP_GET_STATE
};

enum EBankResult {
    RES_OK,
    RES_ERR_LOCKED,
    RES_ERR_UNLOCKED
};

class CBankSupervisor: public CBasicThread {
public:
    CBankSupervisor(std::string name, unsigned size):
        CBasicThread(name),
        m_size(size),
        m_locked(false),
        m_lockOwner(0),
        m_mutex(name + " protect mutex")
    {
        m_inQs.resize(size, NULL);
        m_outQs.resize(size, NULL);
        for (int i = 0; i < size; ++i) {
            std::stringstream ss;
            ss << "queue#" << i;
            m_inQs[i] = new CSafeQueue(std::string("in " + ss.str()));
            m_outQs[i] = new CSafeQueue(std::string("out " + ss.str()));
        }
    }
    void GetQueues(unsigned i, CSafeQueue*& res_inQ, CSafeQueue*& res_outQ) {
        res_inQ = m_inQs[i];
        res_outQ = m_outQs[i];
    }
    CSafeQueue* GetInQPtr(unsigned i) {
        return m_inQs[i];
    }
    CSafeQueue* GetOutQPtr(unsigned i) {
        return m_inQs[i];
    }
    virtual void Body() {
        for (unsigned i = 0; i < m_size; ++i) {
            
        }
    }
protected:
    std::vector<CSafeQueue*> m_inQs;
    std::vector<CSafeQueue*> m_outQs;
    unsigned m_size;
    bool m_locked;
    unsigned m_lockOwner;
    CMutex m_mutex;
};

class CBankUser: public CBasicThread {
public:
    CBankUser(std::string name, std::string filename, CSafeQueue* inQ, CSafeQueue* outQ):
        CBasicThread(name),
        m_filename(filename),
        m_executeProgram(name + " continuation semaphore"),
        m_inQ(inQ),
        m_outQ(outQ)
    {
        assert(m_inQ != NULL || m_outQ != NULL);
    }
    virtual void Body() {
        
    }
protected:
    std::string m_filename;
    CSemaphore m_executeProgram;
    CSafeQueue* m_inQ;
    CSafeQueue* m_outQ;
};

int main()
{
    CBankUser u("1st user", "", NULL, NULL);
    u.Go();
    u.Join();
    return 0;
}
