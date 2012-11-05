#include "mpi_helpers.h"
#include "labyrinth.h"
#include <iostream>

struct CMpiCommunicator: public ICommunicator, public CRankSlave, public MixSlaveLogger, public MixMpiHelper {
    CMpiCommunicator(IHaveRank& rankOwner, ILogger& masterLog):
        CRankSlave(rankOwner),
        MixSlaveLogger(masterLog) {}
    CMpiCommunicator(CMpiConnections c, IHaveRank& rankOwner, ILogger& masterLog):
        CRankSlave(rankOwner),
        MixSlaveLogger(masterLog)
    {
        m_fields[ETop] = c.m_topRank;
        m_fields[ERight] = c.m_rightRank;
        m_fields[EBottom] = c.m_bottomRank;
        m_fields[ELeft] = c.m_leftRank;
    }
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

        SendCmd(CMD_ENTER, 0);
        SendCmd(CMD_GO, rank);
        SendData(value, rank);
        SendData(coord.Flip(), rank);
        return false;
    }
    virtual bool GetWay(uint value, CSideCoord coord, std::string& way) {
        LogEx("Comm GetWay " << coord);
        int rank = m_fields[coord.m_side];
        if (0 == rank) return false;

        SendCmd(CMD_FIND_WAY, rank);
        SendData(value, rank);
        SendData(coord.Flip(), rank);
        SendData(way, rank);

        return false;
    }
};

class CSupervisor: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    CSupervisor(CEnvironment& env, std::string fname): CRankOwner(0), MixTaskLogger(0), m_reader(*this), m_env(env)
    {
        Run(fname);
    }

protected:
    void Run(std::string fname) {
        if (!m_reader.Read(fname)) {
            std::cerr << "Can not read input data\n";
            assert(0);
        }
        { LogEx("Read field\n" << m_reader); }

        int needTasks = m_reader.GetGridSize() * m_reader.GetGridSize() + 1;
        if (m_env.m_procCnt < needTasks) {
            std::stringstream ss;
            ss << "not enough tasks: need " << needTasks << " got " << m_env.m_procCnt;
            throw ss.str();
        } else if (needTasks < m_env.m_procCnt) {
            for (int i = needTasks; i < m_env.m_procCnt; ++i) {
                SendCmd(CMD_DIE, i);
            }
        }
        SendTasks();
        SendGo();
        if (WaitFinish()) {
            NotifyAllTasks(CMD_FIN_FOUND);
            std::string way;
            GetWay(way);
            LogEx("Way found: " << way);
            std::cerr << "Way found: " << way << "\n";
        } else {
            std::cerr << "Can not find way\n";
        }
        NotifyAllTasks(CMD_DIE);
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
                LogEx("to " << task << "\n" << *m_reader.Get(x, y));
            }
        }
    }
    void SendGo() {
        Log("SendGo");
        CPointCoord s  = m_reader.m_startSquareCoord;
        CPointCoord sg = m_reader.m_startGridItem;
        int task = CMpiConnections::GridCoordToRank(m_reader.GetGridSize(), sg);
        SendCmd(CMD_SEND_TASK, task);
        SendData(s, task);
    }
    bool WaitFinish() {
        Log("WaitFinish");
        int delta = 1;
        bool finFound = false;
        ECommands cmd;
        while (delta > 0) {
            cmd = RecieveCmd(MPI_ANY_SOURCE);
            switch (cmd) {
            case CMD_ENTER: {
                ++delta;
            } break;
            case CMD_FIN_FOUND: {
                finFound = true;
            }; // fall throw
            case CMD_EXIT: {
                --delta;
            } break;
            default: assert(0);
            }
            { LogEx("Now delta is " << delta); }
        }
        { LogEx("Work finished " << finFound); }
        return finFound;
    }
    void GetWay(std::string& way) {
        Log("GetWay");
        CPointCoord f  = m_reader.m_finSquareCoord;
        CPointCoord fg = m_reader.m_finGridItem;
        int task = CMpiConnections::GridCoordToRank(m_reader.GetGridSize(), fg);
        SendCmd(CMD_START_FIND_WAY, task);
        SendData(f, task);

        MPI_Status r;
        RecieveAndCheckCmd(CMD_SEND_WAY, MPI_ANY_SOURCE, &r);
        GetData(way, r.MPI_SOURCE);
    }
    void NotifyAllTasks(ECommands cmd) {
        uint s = m_reader.GetGridSize();
        for (uint y = 0; y < s; ++y) {
            for (uint x = 0; x < s; ++x) {
                int rank = CMpiConnections::GridCoordToRank(s, CPointCoord(x, y));                
                SendCmd(cmd, rank);
            }
        }
    }
    CFieldReder m_reader;
    CEnvironment& m_env;
};

class CWorker: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
public:
    CWorker(int rank): CRankOwner(rank), MixTaskLogger(rank), m_field(NULL, *this)
    {
        Run();
    }
protected:
    void Run() {
        bool ok = true;
        bool finFound = false;
        {
            ECommands cmd = RecieveCmd(0);
            switch (cmd) {
            case CMD_SEND_INPUT_DATA: {
                RecieveField();
            } break;
            case CMD_DIE: {
                ok = false;
            } break;
            default: assert(0);
            }
        }

        while (ok && !finFound) {
            MPI_Status r;
            switch(RecieveCmd(MPI_ANY_SOURCE, &r)) {
            case CMD_DIE: {
                ok = false;
            } break;
            case CMD_SEND_TASK: {
                CPointCoord p;
                GetData(p, 0);
                bool res = m_field.Go(0, p);
                if (res) {
                    SendCmd(CMD_FIN_FOUND);
                } else {
                    SendCmd(CMD_EXIT);
                }
            } break;
            case CMD_GO: {
                uint dist;
                CSideCoord coord;
                GetData(dist,  r.MPI_SOURCE);
                GetData(coord, r.MPI_SOURCE);
                bool res = m_field.Go(dist, m_field.GetSize().FromSideCoord(coord));
                if (res) {
                    SendCmd(CMD_FIN_FOUND);
                } else {
                    SendCmd(CMD_EXIT);
                }
            } break;
            case CMD_FIN_FOUND: {
                finFound = true;
            } break;
            case CMD_NONE:
            case CMD_SEND_ANS:
            case CMD_SEND_INPUT_DATA:
            case CMD_NO_FIN:
            case CMD_FIND_WAY:
            case CMD_SEND_WAY:
            case CMD_NO_WAY:
            default: assert(0);
            }
        }

        if (!ok || !finFound) return;

        while (ok) {
            MPI_Status r;
            switch(RecieveCmd(MPI_ANY_SOURCE, &r)) {
            case CMD_START_FIND_WAY: {
                CPointCoord p;
                GetData(p, r.MPI_SOURCE);
                std::string way;
                { LogEx("CMD_START_FIND_WAY\n" << m_field); }
                bool found = m_field.GetWay(-1, p, way);
                if (found) {
                    SendCmd(CMD_SEND_WAY, 0);
                    SendData(way, 0);
                }
            } break;
            case CMD_FIND_WAY: {
                uint dist;
                CSideCoord c;
                std::string way;
                GetData(dist, r.MPI_SOURCE);
                GetData(c, r.MPI_SOURCE);
                GetData(way, r.MPI_SOURCE);
                bool found = m_field.GetWay(dist, m_field.GetSize().FromSideCoord(c), way);
                if (found) {
                    SendCmd(CMD_SEND_WAY, 0);
                    SendData(way, 0);
                }
            } break;
            case CMD_DIE: {
                ok = false;
            } break;
            case CMD_GO: { //skip
                uint dist;
                CSideCoord coord;
                GetData(dist,  r.MPI_SOURCE);
                GetData(coord, r.MPI_SOURCE);
            } break;
            case CMD_NONE:
            case CMD_SEND_TASK:
            case CMD_SEND_ANS:
            case CMD_SEND_INPUT_DATA:
            case CMD_FIN_FOUND:
            case CMD_NO_FIN:
            case CMD_SEND_WAY:
            case CMD_NO_WAY:
            default: assert(0);
            }
        }

        
    }
    void RecieveField() {
        CMpiConnections conn;
        GetData(conn, 0);

        CMatrix matr;
        GetData(matr, 0);
        { LogEx("Got matr\n" << matr); }
        m_field.SetCommunicator(new CMpiCommunicator(conn, *this, *this));
        m_field.GetType() = matr;
        m_field.Resize(matr.m_dim);
        { LogEx("Created field\n" << m_field); }
    }
    CSquareField m_field;
};

int main(int argc, char* argv[])
{
    CEnvironment env;
    env.InitMPI(argc, argv);
    if (argc != 2) {
        if (0 == env.m_rank) {
            std::cerr << "usage: " << argv[0] << " <filename>\n\n";
        }
        return 0;
    }
    
    try {
        if (0 == env.m_rank) {
            CSupervisor s(env, argv[1]);
        } else {
            CWorker w(env.m_rank);
        }
    }
    catch (const char* str) {
        std::cerr << str << "\n\n";
        assert(0);
    }
    catch (std::string str) {
        std::cerr << str << "\n\n";
        assert(0);
    }
    return 0;
}
