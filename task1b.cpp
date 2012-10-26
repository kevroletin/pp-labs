#include <iostream>
#include "Threads.h"
#include <cassert>
#include <sstream>

class CPipe: public CBasicThread {
public:
    CPipe(std::string name): CBasicThread(name), m_in(NULL), m_out(NULL) {}
    virtual void Body() {
        if (m_in != NULL) {
            bool ok = true;
            while (ok) {
                std::stringstream ss;
                CMessage msg = m_in->Pop();
                ss << m_name << " got " << msg.m_str << "\n";
                std::cerr << ss.str();
                if (m_out != NULL) {
                    m_out->Push(msg);
                }
                ok = msg.m_type != 9;
            }
        } else {
            for (int i = 0; i < 10; ++i) {
                std::stringstream ss, ss2;
                ss << m_name << " create " << i << "\n";
                ss2 << i;
                CMessage msg(ss2.str());
                msg.m_type = i;
                std::cerr << ss.str();
                m_out->Push(msg);
            }
        }
    }
    CSafeQueue* m_in;
    CSafeQueue* m_out;
};

class HW: public CBasicThread {
public:
    HW(): CBasicThread("HW") {}
    virtual void Body() {
        std::cerr << "Hello world!!!";
    }
};

int main(int argc, char* argv[])
{
    CPipe* pipeline[5];
    for (int i = 0; i < 5; ++i) {
        std::stringstream ss;
        ss << "Thread#" << i;
        pipeline[i] = new CPipe(ss.str());
    }
    for (int i = 0; i < 4; ++i) {
        pipeline[i]->m_out = pipeline[i+1]->m_in = new CSafeQueue("pin");
    }
    for (int i = 0; i < 5; ++i) {
        pipeline[i]->Go();
    }
    for (int i = 0; i < 5; ++i) {
        pipeline[i]->Join();
    }

    if (0) {
        HW a[10];
        for (int i = 0; i < 10; ++i) a[i].Go();
        for (int i = 0; i < 10; ++i) a[i].Join();
    }
    
    return 0;
}
