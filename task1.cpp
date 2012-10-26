#include <iostream>
#include "Threads.h"
#include <cassert>

CSemaphore s1("s1", 1), s2("s2");

class CThread2: public CBasicThread {
public:
    CThread2(): CBasicThread("Thread 2") {}
    virtual void Body() { 
        for (int i = 0; i < 100; i += 2) {
            LOG_DEBUG("t2 s1 -");
            s1.Get();
            std::cerr << m_name << " - " << i << "\n";
            LOG_DEBUG("t2 s2 +");        
            s2.Put();
        }
    }
};

class CThread3: public CBasicThread {
public:
    CThread3(): CBasicThread("Thread 3") {}
    virtual void Body() { 
        bool bFirst = true;
        for (int i = 1; i < 100; i += 2) {
            LOG_DEBUG("t3 s2 -");
            s2.Get();
            std::cerr << m_name << " - " << i << "\n";
            LOG_DEBUG("t3 s1 +");        
            s1.Put();
        }
    }
};

class CThread1: public CBasicThread {
public:
    CThread1(bool spawnThreads): CBasicThread("Thread 1"), m_spawnThreads(spawnThreads) { }
    virtual void Body() { 
        if (m_spawnThreads) {
            CThread2 t2;
            CThread3 t3;
            t2.Join();
            t3.Join();
        }
    }
protected:
    bool m_spawnThreads;
};

int main(int argc, char* argv[])
{
    switch (1) {
    case 0: {
        CThread1 t1(0);
        CThread2 t2;
        CThread3 t3;
        t2.Join();
        t3.Join();
    } break;
    case 1: {
        CThread1 t1(1);
        t1.Join();
        break;
    }
    }

    std::cerr << "Good\n";
    return 0;
}
