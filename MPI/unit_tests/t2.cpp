#include "../mpi_helpers.h"
#include "../labyrinth.h"
#include <iostream>

class Supervisor: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    Supervisor(int procCnt): CRankOwner(0), MixTaskLogger(0) {
        CFieldReder reader(*this);
        reader.Read("t2/01_in.txt");

        for (int i = 1; i < procCnt; ++i) {
            SendData(-1, i);
            SendData(1u, i);
            SendData(1.1, i);
            SendData(*reader.Get(0, 0), i);
            SendData(CMpiConnections(1, 2, 3, 4), i);
        }
    }
};

struct Worker: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
    uint m_currTest;
    uint m_failed;
    
    void ok(bool res) {
        std::stringstream ss;
        ss << m_rank << ": " << ++m_currTest << " ";
        if (res) {
            ss << "ok";
        } else {
            ss << "fail";
            ++m_failed;
        }
        ss << "\n";
        std::cerr << ss.str();
    }
    Worker(int rank): CRankOwner(rank), MixTaskLogger(rank), m_currTest(0), m_failed(0) {
        {
            int res;
            GetData(res);
            ok(res == -1);
        }
        {
            uint res;
            GetData(res);
            ok(res == 1);
        }
        {
            double res;
            GetData(res);
            ok(res == 1.1);
        }
        {
            CFieldReder reader(*this);
            reader.Read("t2/01_in.txt");

            CSquareField f(NULL, *this);
            GetData(f);
            ok(f == *reader.Get(0, 0));
        }
        {
            CMpiConnections c;
            GetData(c);
            ok(c.m_topRank == 1 && c.m_rightRank == 2 &&
               c.m_bottomRank == 3 && c.m_leftRank == 4);
        }
    }
};
    
int main(int argc, char* argv[])
{
    TEnvironment env;
    env.InitMPI(argc, argv);
    
    if (0 == env.m_rank) {
        Supervisor s(env.m_procCnt);
    } else {
        Worker w(env.m_rank);
    }
    
    return 0;
}
    
