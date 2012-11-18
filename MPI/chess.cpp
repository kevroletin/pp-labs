#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

#define USE_MPI

#undef Log
#undef LogEx
#define Log(str) CLogHelper L(*this, str);
#define LogEx(data)                              \
    {                                            \
        std::stringstream ss;                    \
        ss << data;                              \
        Log(ss.str());                           \
    }


#include "mpi_helpers.h"
#include "chess.h"

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
        if (rank < 4) {
            CMainBoard* b = new CMainBoard(*this, *this);
            if (rank == 1) CBoardConfigure::SetWhiteMainFigures(*b);
            if (rank == 3) CBoardConfigure::SetBlackMainFigures(*b);
            m_board = b;
        } else {
            CAttackBoard* b = new CAttackBoard(*this, *this);
            if (rank == 4) CBoardConfigure::SetWhiteAttack1Figures(*b);
            if (rank == 5) CBoardConfigure::SetWhiteAttack2Figures(*b);
            if (rank == 6) CBoardConfigure::SetBlackAttack1Figures(*b);
            if (rank == 7) CBoardConfigure::SetBlackAttack2Figures(*b);
            m_board = b;
        }
        m_board->m_absoluteCoord = CBoardConfigure::GetInitialCoords(rank);
        Run();
    }
    void Run() {
        LogEx("Run\n" << *m_board);
        bool ok = true;
        while (ok) {
            MPI_Status r;
            ECommands cmd = RecieveCmd(MPI_ANY_SOURCE, &r);
            LogEx("Got " << cmdToStr[cmd]);
            switch(cmd) {
            case CMD_NONE: {
                assert(0);
            } break;
            case CMD_DIE: {
                ok = false;
            } break;
            case CMD_MOVE: {
                CCoord3D from;
                CCoord3D to;
                GetData(from, r.MPI_SOURCE);
                GetData(to, r.MPI_SOURCE);

                if (m_board->CheckMove(from, to, GetRank())) {
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                }
            } break;
            case CMD_DUMP: {
                assert(0); // TODO:
            } break;
            case CMD_IS_CELL_NON_EMPTY: {
                CCoord2D coord;
                GetData(coord, r.MPI_SOURCE);
                CPiece* p = m_board->GetSafe_abs(coord);
                if (NULL == p) {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                }
            } break;
            case CMD_GIVE_CELL_COLOR: {
                CCoord3D coord;
                GetData(coord, r.MPI_SOURCE);
//                LogEx(coord);
                CPiece* p = m_board->GetSafe_abs(coord);
                if (NULL == p) {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                } else {
//                    LogEx(m_board->ToRelativeCoord(coord) << "\n" << *m_board);
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                    SendData(p->m_color, r.MPI_SOURCE);
                }
            } break;
            case CMD_IS_CONTAINS_COORD: {
                CCoord3D coord;
                GetData(coord, r.MPI_SOURCE);
                if (m_board->ContainCoord_abs(coord)) {
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                }
            } break;
            case CMD_SET_PIECE: {
                CCoord3D coord;
                uint pieceType;
                uint color;
                GetData(coord, r.MPI_SOURCE);
                GetData(pieceType, r.MPI_SOURCE);
                GetData(color, r.MPI_SOURCE);
                if (m_board->ContainCoord_abs(coord)) {
                    if (NULL != m_board->Get_abs(coord)) {
                        delete m_board->Get_abs(coord);
                    }
                    m_board->Get_abs(coord) = CreatePiece((EPieces)pieceType);
                    m_board->Get_abs(coord)->m_color = (EColor)color;
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                }
            } break;
            default: assert(0);
            }
        }
        { LogEx("Finish\n" << *m_board); }
    }
    CPiece* CreatePiece(EPieces pieceType) {
        switch(pieceType) {
        case EKnight: {
            return new CKnight;
        } break;
        case EPawn: {
            CPawn* p = new CPawn;
            p->m_firstMove = false;
            return p;
        } break;
        case EQueen: {
            return new CQueen;
        } break;
        case EKing: {
            return new CKing;
        } break;
        case EBishop: {
            return new CBishop;
        } break;
        case ERook: {
            return new CRook;
        } break;
        default: assert(0);
        }
        return NULL;
    }
    CBoard* m_board;
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
