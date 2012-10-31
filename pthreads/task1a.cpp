#include <iostream>
#include "Threads.h"
#include <cassert>

enum ETask {
    ETask1,
    ETask2,
    ETask3
};

std::string taskString[] = {
    "Task1: t1 -> t2 -> t3",
    "Task2: t1 -> t2 | t1 -> t3",
    "Task3: t1 -> t2 | t1 -> t3 + sync"
};

CSemaphore s1("s1", 1), s2("s2");

class CThread3: public CBasicThread {
public:
    CThread3(ETask task):
        CBasicThread("Thread 3"),
        m_task(task) {}
    virtual void Body() { 
        for (int i = 1; i < 100; i += 2) {
            LOG_DEBUG("t3 s2 -");
            if (m_task == ETask3)  s2.Get();
            std::cerr << m_name << " - " << i << "\n";
            LOG_DEBUG("t3 s1 +");        
            if (m_task == ETask3) s1.Put();
        }
    }
protected:
    ETask m_task;
};

class CThread2: public CBasicThread {
public:
    CThread2(ETask task):
        CBasicThread("Thread 2"),
        m_task(task) {}
    virtual void Body() {
        CThread3 t3(m_task);
        if (m_task == ETask1) {
            t3.Go();
        }
        for (int i = 0; i < 100; i += 2) {
            LOG_DEBUG("t2 s1 -");
            if (m_task == ETask3) s1.Get();
            std::cerr << m_name << " - " << i << "\n";
            LOG_DEBUG("t2 s2 +");        
            if (m_task == ETask3) s2.Put();
        }
        if (m_task == ETask1) {
            t3.Join();
        }
    }
protected:
    ETask m_task;
};

class CThread1: public CBasicThread {
public:
    CThread1(ETask task):
        CBasicThread("Thread 1"),
        m_task(task) { }
    virtual void Body() {
        switch (m_task) {
        case ETask1: {
            CThread2 t2(m_task);
            t2.Go();
            t2.Join();
        } break;
        case ETask2:
        case ETask3: {
            CThread2 t2(m_task);
            CThread3 t3(m_task);
            t2.Go();
            t3.Go();
            t2.Join();
            t3.Join();
        } break;
        }
    }
protected:
    ETask m_task;
};

int main(int argc, char* argv[])
{
    if (argc == 2) {
        std::stringstream ss;
        ss << argv[1];
        int i;
        ss >> i;
        ETask task = static_cast<ETask>(i);
        std::cerr << taskString[task] << "\n";
        CThread1 t1(task);
        t1.Go();
        t1.Join();
        std::cerr << "Good\n";
    }

    return 0;
}
