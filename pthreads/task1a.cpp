#include <iostream>
#include "Threads.h"
#include <cassert>

enum ETask {
    ETask1,
    ETask2,
    ETask3
};

std::string taskString[] = {
    "Task1: t1 -> t2 -> t3 + no sync using shared memory",
    "Task2: t1 -> t2 | t1 -> t3 + t1 sync",
    "Task3: t1 -> t2 | t1 -> t3 + self_sync"
};

class CTask1: public CBasicThread {
public:
    CTask1(std::string name, unsigned num_to_print):
        CBasicThread(name),
        m_num(num_to_print) {}
protected:
    virtual void Body() {
        std::cerr << m_name << " - " << m_num << "\n";
    }
    unsigned m_num;
};

class CTask3: public CBasicThread {
public:
    CTask3(std::string name, CSemaphore& s1, CSemaphore& s2, unsigned start_num):
        CBasicThread(name),
        m_start_sem(s1),
        m_fin_sem(s2),
        m_num(start_num) {}
    virtual void Body() {
        for (int i = m_num; i < 100; i += 2) {
            m_start_sem.Get();
            std::cerr << m_name << " - " << i << "\n";
            m_fin_sem.Put();
        }
    }
protected:
    CSemaphore& m_start_sem;
    CSemaphore& m_fin_sem;
    unsigned m_num;
};

class CSpawner: public CBasicThread {
public:
    CSpawner(ETask task):
        CBasicThread("Thread 1"),
        m_task(task) { }
    virtual void Body() {
        bool ok = true;
        switch (m_task) {
        case ETask1: {
            for (int i = 0; i < 100; ++i) {
                std::stringstream ss;
                ss << "Thread " << i;
                CTask1 t(ss.str(), i);
                t.Go();
                t.Join();
            }
        } break;
        case ETask2: {
            CSemaphore s1_start("s11", 0), s1_fin("s12", 0);
            CSemaphore s2_start("s21", 0), s2_fin("s22", 0);
            CTask3 t1("Thread 1", s1_start, s1_fin, 0);
            CTask3 t2("Thread 2", s2_start, s2_fin, 1);
            t1.Go();
            t2.Go();
            int i = 0;
            while (i < 100) {
                s1_start.Put();
                s1_fin.Get(); ++i;
                s2_start.Put();
                s2_fin.Get(); ++i;
            }
        }
        case ETask3: {
            CSemaphore s1("s1", 1), s2("s2");
            CTask3 t1("Thread 1", s1, s2, 0);
            CTask3 t2("Thread 2", s2, s1, 1);
            t1.Go();
            t2.Go();
            t1.Join();
            t2.Join();
        } break;
        default: {
            std::cerr << "Unknown task";
            ok = false;
        }
        }
        if (ok) std::cerr << taskString[m_task] << "\n";
    }
protected:
    ETask m_task;
};

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "argc != 2\n";
    } else {
        std::stringstream ss;
        ss << argv[1];
        int i;
        ss >> i;
        ETask task = static_cast<ETask>(i);
        CSpawner s(task);
        s.Go();
        s.Join();
    }

    return 0;
}
