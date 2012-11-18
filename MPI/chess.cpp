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
    CSupervisor(CEnvironment& env, std::istream& in, std::ostream& out): CRankOwner(0), MixTaskLogger(0), m_env(env)
    {
        Run(in, out);
    }
    void Run(std::istream& in, std::ostream& out) {
        Log("Run");
        char c;
        int xf, yf, levelf;
        int xt, yt, levelt; 
        while( in >> c >> xf >> yf >> levelf >> xt >> yt >> levelt ) {
            bool ok = false;
            for (int i = 1; !ok && i <= maxRank; ++i) {
                if (c == 'M') {
                    SendCmd(CMD_MOVE, i);
                } else {
                    SendCmd(CMD_MOVE_BOARD, i);
                }
                SendData(CCoord3D(xf, yf, levelf), i);
                SendData(CCoord3D(xt, yt, levelt), i);

                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) {
                    ok = true;
                } else {
                    assert(CMD_FAIL == cmd);
                }
            }
            Dump(out);
        }
        for (int i = 1; i <= maxRank; ++i) {
            SendCmd(CMD_DIE, i);
        }
    }
    void Dump(std::ostream& out) {
        for (int i = 1; i <= 3; ++i) {
            SendCmd(CMD_GET_BOARD_INFO, i);
            CCoord3D coord;
            uint color;
            GetData(coord, i);
            GetData(color, i);
            out << "main board#" << i - 1 << ": " << coord << "\n";
        }
        CCoord3D attackCoord[4];
        for (int i = 4; i <= 7; ++i) {
            SendCmd(CMD_GET_BOARD_INFO, i);
            CCoord3D coord;
            uint color;
            GetData(coord, i);
            GetData(color, i);
            out << "attack board#" << i - 4 << ": " << coord << " " << colorToStr[color] << "\n";
            attackCoord[i - 4] = coord;
        }
        for (int y = 0; y < 24; ++y) {
            for (int x = 0; x < 8; ++x) {
                dumpData[y][x] = ' ';
            }
        }
        DumpBoard(1, CCoord2D(2, 2));
        DumpBoard(2, CCoord2D(2, 10));
        DumpBoard(3, CCoord2D(2, 18));

        DumpBoard(4, AttackBoardCooadDelta(attackCoord[0]));
        DumpBoard(5, AttackBoardCooadDelta(attackCoord[1]));
        DumpBoard(6, AttackBoardCooadDelta(attackCoord[2]));
        DumpBoard(7, AttackBoardCooadDelta(attackCoord[3]));

        out << "  ";
        for (int i = 0; i < 6; ++i) {
            out << i;
            if (i == 1 || i == 4) out << i;
        }
        out << "\n";

        static int linum[] = { 0, 1, 1, 2, 3, 4, 4, 5, 2, 3, 3, 4, 5, 6, 6, 7, 4, 5, 5, 6, 7, 8, 8, 9};
        static int level[] = { 2, 0, 1, 0, 0, 0, 2, 0, 4, 0, 3, 0, 0, 0, 4, 0, 6, 0, 5, 0, 0, 0, 6, 0};
        int i = 0;
        for (int y = 0; y < 24; ++y) {
            out << linum[i] << " ";
            for (int x = 0; x < 8; ++x) {
                out << dumpData[y][x];
            }
            if (0 != level[i]) {
                out << " " << level[i] << "\n";
            } else {
                out << "\n";
            }
            ++i;
        }
    }
    void DumpBoard(int rank, CCoord2D p) {
        SendCmd(CMD_DUMP, rank);
        CCoord2D size;
        GetData(size, rank);
        for (int y = 0; y < size.m_y; ++y)
            for (int x = 0; x < size.m_x; ++x) {
                uint pieceType;
                uint color;
                GetData(pieceType, rank);
                GetData(color, rank);
                if (EPieceNone == pieceType) {
                    dumpData[y + p.m_y][x + p.m_x] = '.';
                } else {
                    if (EWhite == color) {
                        dumpData[y + p.m_y][x + p.m_x] = piecesToStr[pieceType][0];
                    } else {
                        dumpData[y + p.m_y][x + p.m_x] = piecesToStrLow[pieceType][0];
                    }
                }
            }
    }
    CCoord2D AttackBoardCooadDelta(CCoord3D coord) {
        CCoord2D p = coord.Get2DPart();
        switch (coord.m_level) {
        case 2: {
            if (p == CCoord2D(0, 0)) return CCoord2D(0, 0);
            if (p == CCoord2D(4, 0)) return CCoord2D(6, 0);
            if (p == CCoord2D(0, 4)) return CCoord2D(0, 6);
            if (p == CCoord2D(4, 4)) return CCoord2D(6, 6);
            assert(0);        
        } break;
        case 4: {
            if (p == CCoord2D(0, 2)) return CCoord2D(0, 8);
            if (p == CCoord2D(4, 2)) return CCoord2D(6, 8);
            if (p == CCoord2D(0, 6)) return CCoord2D(0, 14);
            if (p == CCoord2D(4, 6)) return CCoord2D(6, 14);
            assert(0);        
        } break;
        case 6: {
            if (p == CCoord2D(0, 4)) return CCoord2D(0, 16);
            if (p == CCoord2D(4, 4)) return CCoord2D(6, 16);
            if (p == CCoord2D(0, 8)) return CCoord2D(0, 22);
            if (p == CCoord2D(4, 8)) return CCoord2D(6, 22);
            assert(0);        
        } break;
        default: assert(0);
        }
    }

    char dumpData[24][8];
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

                if (m_board->PerformMove(from, to, GetRank())) {
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                }
            } break;
            case CMD_MOVE_BOARD: {
                CCoord3D from;
                CCoord3D to;
                GetData(from, r.MPI_SOURCE);
                GetData(to, r.MPI_SOURCE);

                if (m_board->MoveBoard(from, to, GetRank())) {
                    SendCmd(CMD_OK, r.MPI_SOURCE);
                } else {
                    SendCmd(CMD_FAIL, r.MPI_SOURCE);
                }                
            } break;
            case CMD_DUMP: {
                CCoord2D size = m_board->GetSize();
                SendData(size, r.MPI_SOURCE);
                for (int y = 0; y < size.m_y; ++y)
                    for (int x = 0; x < size.m_x; ++x) {
                        if (NULL != m_board->Get(x, y)) {
                            SendData(m_board->Get(x, y)->GetPieceTipe(), r.MPI_SOURCE);
                            SendData((uint)m_board->Get(x, y)->m_color, r.MPI_SOURCE);
                        } else {
                            SendData(EPieceNone, r.MPI_SOURCE);
                            SendData(ENone, r.MPI_SOURCE);
                        }
                    }
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
            case CMD_GET_BOARD_INFO: {
                SendData(m_board->m_absoluteCoord, r.MPI_SOURCE);
                SendData((uint)m_board->GetColor(), r.MPI_SOURCE);
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
                CSupervisor s(env, fin, std::cerr);
                s.PublishLog();
            } else if (argc == 3) {
                std::ifstream fin;
                std::ofstream fout;
                fin.open(argv[1]);
                fout.open(argv[2]);
                CSupervisor s(env, fin, fout);
                s.PublishLog();
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
    
