#include "mpi_helpers.h"
#include "labyrinth.h"
#include <iostream>

struct CMpiCommunicator: public ICommunicator, public CRankSlave, public MixSlaveLogger, public MixMpiHelper {
    CMpiCommunicator(IHaveRank& rankOwner, ILogger& masterLog):
        CRankSlave(rankOwner),
        MixSlaveLogger(masterLog) {}
    CMpiCommunicator(int top, int right, int bottom, int left, IHaveRank& rankOwner, ILogger& masterLog):
        CRankSlave(rankOwner),
        MixSlaveLogger(masterLog)
    {
        m_fields[ETop] = top;
        m_fields[ERight] = right;
        m_fields[EBottom] = bottom;
        m_fields[ELeft] = left;
    }
    int m_fields[4];
    virtual bool Go(uint value, CSideCoord coord) {
        LogEx("Comm Go " << coord);
        int rank = m_fields[coord.m_side];
        if (0 == rank) return false;

        SendCmd(CMD_GO, rank);
        SendData(value, rank);
        SendData(coord.Flip(), rank);
        return false;
    }
    virtual bool GetWay(uint value, CSideCoord coord, std::string& way) {
        LogEx("Comm GetWay " << coord);
        LogEx("Comm Go " << coord);
        int rank = m_fields[coord.m_side];
        if (0 == rank) return false;

        SendCmd(CMD_FIND_WAY, rank);
        SendData(value, rank);
        SendData(coord.Flip(), rank);
        uint cmd = RecieveCmd();
        if (CMD_NO_WAY == cmd) {
            // TODO:
        } else if (CMD_SEND_WAY == cmd) {
            // TODO:
        }
        assert(0);
        return false;
    }
};

class CSupervisor: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    CSupervisor(std::string fname): CRankOwner(0), MixTaskLogger(0), m_reader(*this)
    {
        Run(fname);
    }

protected:
    void Run(std::string fname) {
        if (!m_reader.Read(fname)) {
            std::cerr << "Can not read input data\n";
            assert(0);
        }

        SendTasks();
        WaitFinish();
        GetWay();
    }
    void SendTasks() {
        Log("SendTasks");
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
                SendCmd(CMD_SEND_INPUT_DATA, task);
                SendData(conn, task);
                SendData(m_reader.Get(x, y)->GetType(), task);
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

class CWorker: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    CWorker(int rank): CRankOwner(rank), MixTaskLogger(rank)
    {
        Run();
    }
protected:
    void Run() {
        CMpiConnections conn;
        GetData(conn);
        CMatrix matr;
        GetData(matr);
//        { LogEx("Got matr\n" << matr); }
        
        CSquareField field(NULL, *this);

        field.GetType() = matr;
        field.Resize(matr.m_dim);
        { LogEx("Created field\n" << field); }
    }
};

int main(int argc, char* argv[])
{
    CEnvironment env;
    env.InitMPI(argc, argv);

    if (0 == env.m_rank) {
        CSupervisor s("unit_tests/t1/01_in.txt");
    } else {
        CWorker w(env.m_rank);
    }
    return 0;
}
