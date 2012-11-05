#include "mpi_helpers.h"
#include "labyrinth.h"
#include <iostream>

class CSupervisor: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    CSupervisor(): CRankOwner(0), MixTaskLogger(0), m_reader(*this) {}

    void Run(std::string fname) {
        if (!m_reader.Read(fname)) {
            std::cerr << "Can not read input data\n";
            assert(0);
        }

        SendTasks();
        WaitFinish();
        GetWay();
    }
protected:
    void SendTasks() {
        Log("SendData");
        uint s = m_reader.GetGridSize();
        for (uint y = 0; y < s; ++y) {
            for (uint x = 0; x < s; ++x) {
                CMpiConnections conn = CMpiConnections(
                    s,
                    CPointCoord(x,   y-1),
                    CPointCoord(x+1, y  ),
                    CPointCoord(x,   y+1),
                    CPointCoord(x-1, y  ) );
                int task = conn.GridCoordToRank(s, CPointCoord(x, y));
                SendData(conn, task);
                SendData(*m_reader.Get(x, y), task);
            }
        }
    }
    void WaitFinish() {
        Log("WaitFinish");
    }
    void GetWay() {
        Log("GetWay");
    }
    CFieldReder m_reader;
};

class CWorker: public CRankOwner, public MixTaskLogger {
public:
    CWorker(int rank): CRankOwner(rank), MixTaskLogger(rank) {}
};

int main(int argc, char* argv[])
{
    
    return 0;
}
