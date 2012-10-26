#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <queue>
#include "Threads.h"
#include <cassert>

enum EBankOperation {
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_LOCK,
    OP_UNLOCK,
    OP_GET_STATE,
    OP_FINISH,
    OP_UNDEFINED
};

std::string EBankOperationToStr[] = {
    "OP_NONE",
    "OP_ADD",
    "OP_SUB",
    "OP_LOCK",
    "OP_UNLOCK",
    "OP_GET_STATE",
    "OP_FINISH",
    "OP_UNDEFINED"
};

std::string EBankOperationToStrShort[] = {
    "",
    "+",
    "-",
    "L",
    "U",
    "S",
    "F",
    "OP_UNDEFINED"
};

enum EBankResult {
    RES_OK,
    RES_ERR_LOCKED,
    RES_ERR_UNLOKED,
    RES_NOW_LOCK_YOUR,
    RES_PENDING_LOCK,
    RES_ERR_NOT_OWNER,
    RES_UNDEFINED
};

std::string EBankResultToStr[] = {
    "RES_OK",
    "RES_ERR_LOCKED",
    "RES_ERR_UNLOKED",
    "RES_NOW_LOCK_YOUR",
    "RES_PENDING_LOCK",
    "RES_ERR_NOT_OWNER",
    "RES_UNDEFINED"
};

struct CBankOp {
    CBankOp(): m_op(), m_data() {}
    CBankOp(EBankOperation op, unsigned data = -1): m_op(op), m_data(data) {}
    CBankOp(std::string str, unsigned data = -1): m_op(OP_UNDEFINED), m_data(data) {
        for (int i = 0; i < OP_UNDEFINED && m_op == OP_UNDEFINED; ++i) {
            if (EBankOperationToStrShort[i] == str) {
                m_op = static_cast<EBankOperation>(i);
            }
        }
    }
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

std::ostream& operator<<(std::ostream& out, CBankOp& op) {
    out << op.ToStr();
}

struct CBankRes {
    CBankRes(): m_res(RES_UNDEFINED) {}
    CBankRes(EBankResult res): m_res(res) {}
    EBankResult m_res;
    std::string ToStr() {
        std::stringstream ss;
        ss << EBankResultToStr[m_res];
        return ss.str();
    }
};

typedef CQueue<CBankOp> TOpQ;
typedef CQueue<CBankRes> TResQ;

class CBankSupervisor: public CBasicThread {
public:
    CBankSupervisor(std::string name, unsigned size):
        CBasicThread(name),
        m_size(size),
        m_locked(false),
        m_lockOwner(-1),
        m_mutex(name + " protect mutex"),
        account(0)
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
    unsigned GetAccount() { return account; }
protected:
    std::vector<TOpQ*> m_opQs;
    std::vector<TResQ*> m_resQs;
    std::queue<unsigned> waitQueue;
    unsigned m_size;
    bool m_locked;
    unsigned m_lockOwner;
    CMutex m_mutex;
    unsigned account;

    virtual void Body() {
        CBankOp op;
        unsigned finCnt = 0;
        while (finCnt < m_size) {
            bool empty = true;
            for (unsigned i = 0; i < m_size; ++i) {
                if (m_opQs[i]->TryPop(op)) {
                    NAMED_DEBUG("get operation " + op.ToStr());
                    ProcessMessage(i, op, finCnt);
                    empty = false;
                }
            }
            if (empty) {
                NAMED_DEBUG("sleep");
                usleep(10000);
            }
        }
    }
    void SendRes(unsigned client, EBankResult msg) {
        m_resQs[client]->Push(EBankResult(msg));
    }
    void ProcessMessage(unsigned client, CBankOp& op, unsigned& finCnt) {
        switch (op.m_op) {
        case OP_NONE: {
            assert(0);
        } break;
        case OP_ADD:
        case OP_SUB: {
            if (!m_locked) {
                SendRes(client, RES_ERR_UNLOKED);
            } else {
                if (m_lockOwner == client) { //perform
                    if (op.m_op == OP_ADD) {
                        account += op.m_data;
                    } else {
                        account -= op.m_data;
                    }
                    SendRes(client, RES_OK);
                } else {
                    SendRes(client, RES_ERR_LOCKED);
                }
            }
        } break;
        case OP_LOCK: {
            if (!m_locked) {
                m_locked = true;
                m_lockOwner = client;
                SendRes(client, RES_NOW_LOCK_YOUR);
            } else if (op.m_data && m_lockOwner != client) { //wait
                waitQueue.push(client);
                SendRes(client, RES_PENDING_LOCK);
            } else {
                SendRes(client, RES_ERR_LOCKED);
            }
        } break;
        case OP_UNLOCK: {
            if (m_locked && m_lockOwner == client) {
                SendRes(client, RES_OK);
                if (!waitQueue.empty()) {
                    unsigned newOwner = waitQueue.front(); waitQueue.pop();
                    m_lockOwner = newOwner;
                    SendRes(newOwner, RES_NOW_LOCK_YOUR);
                } else {
                    m_locked = false;
                    m_lockOwner = -1;
                }
            } else if (m_locked && m_lockOwner != client) {
                SendRes(client, RES_ERR_NOT_OWNER);
            } else {
                SendRes(client, RES_ERR_UNLOKED);
            }
        } break;
        case OP_GET_STATE: {
            // TODO:
        } break;
        case OP_FINISH: {
            ++finCnt;
            SendRes(client, RES_OK);
        } break;
        case OP_UNDEFINED: {
            assert(0); // ^_^
        } break;
        }
    }
};

class CBankUser: public CBasicThread {
public:
    CBankUser(std::string name, std::string filename, TOpQ* opQ, TResQ* resQ):
        CBasicThread(name),
        m_filename(filename),
        m_actionAllowed(name + " act allowed sem"),
        m_actionCompleted(name + " act completed sem"),
        m_opQ(opQ),
        m_resQ(resQ),
        m_skipMessages(0)
    {
        assert(m_opQ != NULL || m_resQ != NULL);
    }
    void DumpProgramm(std::ostream& out) {
        PROTECT;
        for (std::vector<CBankOp>::iterator it = m_programm.begin(); it != m_programm.end(); ++it) {
            out << *it << "\n";
        }
    }
    void WaitStep() {
        m_actionCompleted.Get();
    }
    void AllowNextStep() {
        NAMED_INFO("allowed next step");
        m_actionAllowed.Put();
    }
protected:
    std::string m_filename;
    CSemaphore m_actionAllowed;
    CSemaphore m_actionCompleted;
    TOpQ* m_opQ;
    TResQ* m_resQ;
    std::vector<CBankOp> m_programm;
    unsigned m_skipMessages;
    void NextStep() {
        NAMED_INFO("next step");
        m_actionCompleted.Put();
        m_actionAllowed.Get();
    }
    virtual void Body() {
        ReadProgram();
        DumpProgrammToLog();
        NextStep();
        ExecuteProgramm();
    }
    bool ReadProgram() {
        NAMED_INFO("read programm");
        std::ifstream in;
        in.open(m_filename.c_str());
        std::string str;
        while (in >> str) {
            CBankOp bankOp(str);
            NAMED_DEBUG(str);
            if (bankOp.m_op != OP_GET_STATE && bankOp.m_op != OP_FINISH) {
                in >> bankOp.m_data;
            }
            m_programm.push_back(bankOp);
        }
    }
    void DumpProgrammToLog() {
        PROTECT;
        NAMED_INFO("programm dump");
        for (std::vector<CBankOp>::iterator it = m_programm.begin(); it != m_programm.end(); ++it) {
            NAMED_INFO(it->ToStr());
        }
    }
    void ExecuteProgramm() {
        for (std::vector<CBankOp>::iterator it = m_programm.begin(); it != m_programm.end(); ++it) {
            NAMED_INFO("sendind " + it->ToStr());
            m_opQ->Push(*it);
            CBankRes res;
            do {
                res = m_resQ->Pop();
                NAMED_INFO("got " + res.ToStr());
            } while (res.m_res == RES_PENDING_LOCK);
        }
        
    }
};

int main(int argc, char* argv[])
{
    CBankSupervisor s("Supervisor", argc - 1);
    CBankUser* u[100];
    for (int i = 0; i < argc - 1; ++i) {
        std::stringstream ss;
        ss << "User#" << i;
        u[i] = new CBankUser(ss.str(), argv[i + 1], s.GetOpQPtr(i), s.GetResQPtr(i));
    }
    s.Go();
    for (int i = 0; i < argc - 1; ++i) u[i]->Go();
    for (int i = 0; i < argc - 1; ++i) u[i]->WaitStep();
    for (int i = 0; i < argc - 1; ++i) u[i]->AllowNextStep();
    s.Join();
    for (int i = 0; i < argc - 1 ; ++i) u[i]->Join();
    s.PublishLog(std::cerr);
    for (int i = 0; i < argc - 1; ++i) {
        std::stringstream ss;
        ss << argv[i] << "." << i << ".log";
        std::ofstream fout;
        fout.open(ss.str().c_str());
        u[i]->PublishLog(fout);
        fout.close();
    }        
    std::cerr << "Good\n" << s.GetAccount();
    return 0;
}
