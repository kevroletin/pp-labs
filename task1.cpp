#include <iostream>
#include "Threads.h"

int main(int argc, char* argv[])
{
    CBasicThread t("TestThread");
    t.Join();
    t.PublishLog(std::cerr);
    if (0) {
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
    }

    
    std::cerr << "Good\n";
    g_semaphoreLog.PublishLog(std::cerr);
    return 0;
}
