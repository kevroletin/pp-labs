#include <iostream>
#include <string>
#include "Threads.h"
#include <cassert>

enum EBankOperation {
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_LOCK,
    OP_UNLOCK,
    OP_GET_STATE
};

std::string EBankOperationToStr[] = {
    "OP_NONE",
    "OP_ADD",
    "OP_SUB",
    "OP_LOCK",
    "OP_UNLOCK",
    "OP_GET_STATE"
};

enum EBankResult {
    RES_OK,
    RES_PENDING_LOCK,
    RES_ERR_UNLOCKED,
    RES_UNDEFINED
};

std::string EBankResultToStr[] = {
    "RES_OK",
    "RES_PENDING_LOCK",
    "RES_ERR_UNLOCKED",
    "RES_UNDEFINED"
};

struct CBankOp {
    CBankOp(): m_op(), m_data() {}
    CBankOp(EBankOperation op, unsigned data = -1): m_op(op), m_data(data) {}
    EBankOperation m_op;
    unsigned m_data;
    std::string ToStr() {
        std::stringstream ss;
        ss << EBankOperationToStr[m_op] << " ";
        if (m_data == -1) {
            ss << "<dummy>";
        } else {
            ss << m_data;
        }
        return ss.str();
    }
};

struct CBankRes {
    CBankRes(): m_res(RES_UNDEFINED) {}
    CBankRes(EBankResult res): m_res(res) {}
    EBankResult m_res;
};

typedef CQueue<CBankOp> TOpQ;
typedef CQueue<CBankRes> TResQ;

class CBankSupervisor: public CBasicThread {
public:
    CBankSupervisor(std::string name, unsigned size):
        CBasicThread(name),
        m_size(size),
        m_locked(false),
        m_lockOwner(0),
        m_mutex(name + " protect mutex")
    {
        m_opQs.resize(size, NULL);
        m_resQs.resize(size, NULL);
        for (int i = 0; i < size; ++i) {
            std::stringstream ss;
            ss << "queue#" << i;
            m_opQs[i] = new TOpQ(std::string("op " + ss.str()));
            m_resQs[i] = new TResQ(std::string("res " + ss.str()));
        }
    }
    void GetQueues(unsigned i, TOpQ*& res_opQ, TResQ*& res_resQ) {
        PROTECT;
        res_opQ = m_opQs[i];
        res_resQ = m_resQs[i];
    }
    TOpQ* GetOpQPtr(unsigned i) {
        PROTECT;
        return m_opQs[i];
    }
    TResQ* GetResQPtr(unsigned i) {
        PROTECT;
        return m_resQs[i];
    }
protected:
    std::vector<TOpQ*> m_opQs;
    std::vector<TResQ*> m_resQs;
    unsigned m_size;
    bool m_locked;
    unsigned m_lockOwner;
    CMutex m_mutex;

    virtual void Body() {
        CBankOp op;
        int cnt = 0;
        while (cnt++ < 5) {
            bool empty = true;
            for (unsigned i = 0; i < m_size; ++i) {
                if (m_opQs[i]->TryPop(op)) {
                    LOG_DEBUG(m_name + " get operation " + op.ToStr());
                    empty = false;
                }
            }
            if (empty) {
                LOG_DEBUG(m_name + " sleep");
                usleep(100);
            }
        }
    }
};

class CBankUser: public CBasicThread {
public:
    CBankUser(std::string name, std::string filename, TOpQ* opQ, TResQ* resQ):
        CBasicThread(name),
        m_filename(filename),
        m_executeProgram(name + " continuation semaphore"),
        m_opQ(opQ),
        m_resQ(resQ)
    {
        assert(m_opQ != NULL || m_resQ != NULL);
    }
protected:
    std::string m_filename;
    CSemaphore m_executeProgram;
    TOpQ* m_opQ;
    TResQ* m_resQ;
    virtual void Body() {
        m_opQ->Push(CBankOp(OP_LOCK));
    }
};

int main()
{
    CBankSupervisor s("Supervisor", 1);
    CBankUser u(std::string("1st User"), std::string(""), s.GetOpQPtr(0), s.GetResQPtr(0));
    u.Go();
    s.Go();

    sleep(1);
    
    s.Join();
    u.Join();
    s.PublishLog(std::cerr);
    u.PublishLog(std::cerr);
    std::cerr << "Good";
    return 0;
}
