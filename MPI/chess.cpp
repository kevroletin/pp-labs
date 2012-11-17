#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

#define USE_MPI
#define CHESS

#include "mpi_helpers.h"
#include "chess.h"

#define Log(str) CLogHelper L(*this, str);
#define LogEx(data)                              \
    {                                            \
        std::stringstream ss;                    \
        ss << data;                              \
        Log(ss.str());                           \
    }
#if 0
#    define AlgoLog(str) Log(str)
#    define AlgoLogEx(data) LogEx(data)
#else
#    define AlgoLog(str)
#    define AlgoLogEx(data)
#endif

const int maxRank = 7;

struct CSupervisor: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
    CSupervisor(CEnvironment& env, std::istream& in): CRankOwner(0), MixTaskLogger(0), m_env(env)
    {
        Run(in);
    }
    void Run(std::istream& in) {
        Log("Run");
        char c;
        int xf, yf, levelf;
        int xt, yt, levelt; 
        while( in >> c >> xf >> yf >> levelf >> xt >> yt >> levelt ) {
            bool ok = false;
            for (int i = 1; !ok && i <= maxRank; ++i) {
                if (c == 'M') {
                    SendCmd(CMD_MOVE, i);
                    SendData(CCoord3D(xf, yf, levelf), i);
                    SendData(CCoord3D(xt, yt, levelt), i);
                } else {
                    // TODO;
                }
                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) {
                    ok = true;
                } else {
                    assert(CMD_FAIL == cmd);
                }
            }
        }
        for (int i = 1; i <= maxRank; ++i) {
            SendCmd(CMD_DIE, i);
        }
    }
    CEnvironment& m_env;
};

struct CWorker: public CRankOwner, public MixMpiHelper, public MixTaskLogger {
    CWorker(int rank): CRankOwner(rank), MixTaskLogger(rank)
    {
        Run();
    }
    void Run() {
        Log("Run");
        bool ok = true;
        while (ok) {
            MPI_Status r;
            ECommands cmd = RecieveCmd(MPI_ANY_SOURCE, &r);
            LogEx(cmd);
            switch(cmd) {
            case CMD_NONE: {
                assert(0);
            } break;
            case CMD_DIE: {
                ok = false;
            } break;
            case CMD_MOVE: {
                // TODO:
                CCoord3D from;
                CCoord3D to;
                GetData(from, r.MPI_SOURCE);
                GetData(to, r.MPI_SOURCE);
                SendCmd(CMD_FAIL, r.MPI_SOURCE);
            } break;
            default: assert(0);
            }
        }
    }
};
    
    
int main(int argc, char* argv[])
{
    CEnvironment env;
    env.InitMPI(argc, argv);
    
    try {
        if (env.m_procCnt - 1 < maxRank) throw "need 8 threads";
        if (0 == env.m_rank) {
            if (argc == 2) {
                std::ifstream fin;
                fin.open(argv[1]);
                CSupervisor s(env, fin);
                s.PublishLog();
            } else {
//                CSupervisor s(env, std::cin);
            }
        } else {
            if (env.m_rank <= maxRank) {
                CWorker w(env.m_rank);
                w.PublishLog();
            }
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
    
#if 0

* Coding conventions

1) All calls are syncronious. Even in multithread environments: method
call should be replaced to message sending; response should be sent
for every message.

2) Because of 1) initial version of programm can be written for
single-thread environment, debbuged, and then ported to multy thread
environments.

* Project plan

** Code structure
There will be 8 threads: supervisor, 3 main boards, 4 attack boards.

*Boards* represents grid of cells; each cell on every board have
relative coordinates and absolute coordinates.

** Move

1) Supervisor determine board which owns needed figure and send
2 absolute coords: fromCorod, toCoord. Board using figure make move
plan which contains 2d relative ccords. On each move step it asks
toBoard is it have such 2d coords with some level coord value.


#endif
