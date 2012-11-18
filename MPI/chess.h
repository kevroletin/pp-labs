#ifndef _CHESS_H
#define _CHESS_H

#include "common.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

const int maxRank = 7;

static inline int sign(int x) { return x > 0 ? 1 : x == 0 ? 0 : -1; }
static inline void printSpaces(std::ostream& out, int x) { while(x--) { out << ' '; } }

struct CCoord2D {
    CCoord2D(int x = 0, int y = 0): m_x(x), m_y(y) {}
    bool Inside(CCoord2D p) {
        return p.m_x >= 0 && p.m_y >= 0 && p.m_x < m_x && p.m_y < m_y;
    }
    CCoord2D operator-(CCoord2D p) { return CCoord2D(m_x - p.m_x, m_y - p.m_y); }
    CCoord2D operator+(CCoord2D p) { return CCoord2D(m_x + p.m_x, m_y + p.m_y); }
    bool operator==(CCoord2D p) { return m_x == p.m_x && m_y == p.m_y; }
    bool operator!=(CCoord2D p) { return m_x != p.m_x || m_y != p.m_y; }
    
    int m_x;
    int m_y;
};

static inline std::ostream& operator<<(std::ostream& out, CCoord2D c) {
    out << "x:" << c.m_x << " y:" << c.m_y;
    return out;
}

struct CCoord3D {
    CCoord3D(int x = 0, int y = 0, int level = 0): m_x(x), m_y(y), m_level(level) {}
    CCoord2D Get2DPart() { return CCoord2D(m_x, m_y); }
    CCoord3D operator-(CCoord2D p) { return CCoord3D(m_x - p.m_x, m_y - p.m_y, m_level); }
    CCoord3D operator+(CCoord2D p) { return CCoord3D(m_x + p.m_x, m_y + p.m_y, m_level); }
    CCoord3D operator-(CCoord3D p) { return CCoord3D(m_x - p.m_x, m_y - p.m_y, m_level - p.m_level); }
    CCoord3D operator+(CCoord3D p) { return CCoord3D(m_x + p.m_x, m_y + p.m_y, m_level + p.m_level); }
    bool operator==(CCoord3D p) { return m_x == p.m_x && m_y == p.m_y && m_level == p.m_level; }
    bool operator!=(CCoord3D p) { return m_x != p.m_x || m_y == p.m_y || m_level == p.m_level; }
    int m_x;
    int m_y;
    int m_level;
};

static inline std::ostream& operator<<(std::ostream& out, CCoord3D c) {
    out << "x:" << c.m_x << " y:" << c.m_y << " level:" << c.m_level;
    return out;
}

enum EColor {
    ENone = 0,
    EWhite = 1,
    EBlack = 2,
};

static std::string colorToStr[] = { "none", "white", "black" };

/*enum ESide {
    ETop,
    ERight,
    EBottom,
    ELeft,
    };*/

enum EPinSide {
    ELeftTop,
    ERightTop,
    ERightBottom,
    ELeftBottom,
};

enum EPieces {
    EKnight, //Конь
    EPawn,   //Пешка
    EQueen,
    EKing,
    EBishop, //Офицер
    ERook,   //Ладья
    EPieceNone,
};


static std::string piecesToStr[] = { "H", "P", "Q", "K", "B", "R" };
static std::string piecesToStrLow[] = { "h", "p", "q", "k", "b", "r" };

struct CMove {
    CMove(int dx, int dy, bool jump = false): m_dx(dx), m_dy(dy), m_jump(jump) {}
    CMove(CCoord2D c, bool jump = false): m_dx(c.m_x), m_dy(c.m_y), m_jump(jump) {}
    int m_dx;
    int m_dy;
    bool m_jump;
};

inline CCoord2D operator+(CCoord2D c, CMove m) {
    c.m_x += m.m_dx;
    c.m_y += m.m_dy;
    return c;
}

inline CCoord3D operator+(CCoord3D c, CMove m) {
    c.m_x += m.m_dx;
    c.m_y += m.m_dy;
    return c;
}

static inline std::ostream& operator<<(std::ostream& out, CMove move) {
    if (move.m_jump) out << "*";    
    out << "dx:" << move.m_dx << " dy:" << move.m_dy;
    return out;
}

typedef std::vector<CMove> T2DPath;
typedef std::vector<CCoord3D> T3DPath;

static inline std::ostream& operator<<(std::ostream& out, const T2DPath& path) {
    T2DPath::const_iterator it = path.begin();
    if (it != path.end()) {
        out << *it;
        ++it;
    }
    for (; it != path.end(); ++it) {
        out << "->" << *it;
    }
    return out;
}

struct CPiece {
    CPiece(): m_color(EWhite) {}
    EColor m_color;
    virtual EPieces GetPieceTipe() = 0;
    virtual T2DPath PlanMove(CCoord2D relativeCoords) = 0;
    virtual void BeforeMove() {}
};

static inline std::ostream& operator<<(std::ostream& out, CPiece& p) {
    std::string str = piecesToStr[p.GetPieceTipe()];
    if (EBlack == p.m_color) {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
    }
    if (ENone == p.m_color) { // DEBUG
        str = "x";
    }
    out << str;
    return out;
}

struct CKnight: public CPiece {
    virtual EPieces GetPieceTipe() { return EKnight; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        if (c == CCoord2D( 2,  1) || c == CCoord2D( 2, -1) || c == CCoord2D(-2,  1) || c == CCoord2D(-2, -1) ||
            c == CCoord2D( 1,  2) || c == CCoord2D(-1,  2) || c == CCoord2D( 1, -2) || c == CCoord2D(11, -2))
        {
            CCoord2D p(0, 0);
            for (int x = 0; x != c.m_x; x += sign(c.m_x)) {
                path.push_back(CMove(sign(c.m_x), 0, true));
            }
            for (int y = 0; y != c.m_y; y += sign(c.m_y)) {
                path.push_back(CMove(0, sign(c.m_y), true));
            }
        }
        return path;
    }
};

struct CPawn: public CPiece {
    CPawn(): m_firstMove(true) {}
    bool m_firstMove;
    virtual EPieces GetPieceTipe() { return EPawn; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        int forward = m_color == EWhite ? 1 : -1;
        if (c.m_x != 0) return path;
        if (c.m_y == forward) {
            path.push_back(c);
        } else if (m_firstMove && c.m_y == 2*forward) {
            path.push_back(CMove(0, forward));
            path.push_back(CMove(0, forward));
        }
        return path;
    }
    virtual void BeforeMove() { m_firstMove = false; }
};

struct CKing: public CPiece {
    virtual EPieces GetPieceTipe() { return EKing; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        if (abs(c.m_x) == 1 && abs(c.m_y) <= 1) {
            path.push_back(c);
        }
        return path;
    }
};

struct CBishop: public CPiece {
    virtual EPieces GetPieceTipe() { return EBishop; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        if (abs(c.m_x) == abs(c.m_y)) {
            CCoord2D p(0, 0);
            while (p != c) {
                p.m_x += sign(c.m_x);
                p.m_y += sign(c.m_y);
                path.push_back(CMove(sign(c.m_x), sign(c.m_y)));                
            }
        }
        return path;
    }
};

struct CRook: public CPiece {
    virtual EPieces GetPieceTipe() { return ERook; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        if (0 == c.m_x || 0 == c.m_y) {
            CCoord2D p(0, 0);
            while (p != c) {
                p.m_x += sign(c.m_x);
                p.m_y += sign(c.m_y);
                path.push_back(CMove(sign(c.m_x), sign(c.m_y)));
            }
        }
        return path;
    }
};

struct CQueen: public CPiece {
    virtual EPieces GetPieceTipe() { return EQueen; }
    virtual T2DPath PlanMove(CCoord2D c) {
        T2DPath path;
        if (0 == c.m_x || 0 == c.m_y || abs(c.m_x) == abs(c.m_y)) {
            CCoord2D p(0, 0);
            while (p != c) {
                p.m_x += sign(c.m_x);
                p.m_y += sign(c.m_y);
                path.push_back(CMove(sign(c.m_x), sign(c.m_y)));
            }
        }
        return path;
    }
};

struct CBoardBroadcast {
    virtual bool AnyCellNonEmpty_abs(CCoord2D absCoord) = 0;
    virtual EColor CellColor(CCoord3D absCoord) = 0;
    virtual bool ContainCoord_abs(CCoord3D absCoord) = 0;
};

#ifdef USE_MPI
struct CBoard: public CRankSlave, public MixMpiHelper, public MixSlaveLogger {
    CBoard(IHaveRank& owner, ILogger& log): CRankSlave(owner), MixSlaveLogger(log) {}
#else
struct CBoard {
#endif
    static const int maxBoardSize = 4;
    
    int m_level;
    CCoord3D m_absoluteCoord;
    CPiece* m_field[maxBoardSize][maxBoardSize];
    virtual EColor GetColor() { return ENone; }
    void Init() {
        CCoord2D size = GetSize();
        for (int y = 0; y < size.m_y; ++y) {
            for (int x = 0;  x < size.m_x; ++x) {
                Get(CCoord2D(x, y)) = NULL;
            }
        }
    }
    virtual CCoord2D GetSize() = 0;
    bool ContainCoord_abs(CCoord3D coord) {
        return coord.m_level == m_absoluteCoord.m_level &&
               GetSize().Inside(ToRelativeCoord(coord));
    }
#ifdef USE_MPI
    bool MPI_ContainCoord_abs(int rank, CCoord3D coord) {
        for (int i = 1; i < maxRank; ++i) {
            if (i != rank) {
                SendCmd(CMD_IS_CONTAINS_COORD, i);
                SendData(coord, i);
                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) return true;
                assert(CMD_FAIL == cmd);
            }
        }
        return false;
    }
    EColor MPI_CellColor(int rank, CCoord3D absCoord) {
        Log("MPI_CellColor");
        for (int i = 1; i < maxRank; ++i) {
            if (i != rank) {
                SendCmd(CMD_GIVE_CELL_COLOR, i);
                SendData(absCoord, i);
                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) {
                    uint color;
                    GetData(color, i);
                    LogEx("MPI_CellColor " << colorToStr[color] << " from " << i);
                    return (EColor) color;
                }
            }
        }        
        LogEx("MPI_CellColor ENone");
        return ENone;
    }
    bool MPI_AnyCellNonEmpty_abs(int rank, CCoord2D absCoord) {
        for (int i = 1; i < maxRank; ++i) {
            if (i != rank) {
                SendCmd(CMD_IS_CELL_NON_EMPTY, i);
                SendData(absCoord, i);
                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) {
                    return true;
                }
                assert(CMD_FAIL == cmd);
            }
        }        
        return false;
    }
    bool MPI_SetPiece(int rank, CCoord3D absCoord, EPieces pieceType, EColor color) {
        for (int i = 1; i < maxRank; ++i) {
            if (i != rank) {
                SendCmd(CMD_SET_PIECE, i);
                SendData(absCoord, i);
                SendData(pieceType, i);
                SendData((uint)color, i);
                ECommands cmd = RecieveCmd(i);
                if (CMD_OK == cmd) {
                    return true;
                }
                assert(CMD_FAIL == cmd);
            }
        }        
        return false;
    }
#endif
    bool ContainCoord_abs(CCoord2D coord) {
        CCoord2D size = GetSize();
        return size.Inside(ToRelativeCoord(coord));
    }
    bool ContainCoord(CCoord2D coord) {
        return GetSize().Inside(coord);
    }
    CCoord2D GetCornerCoord(EPinSide side = ELeftTop) {
        CCoord2D c = GetSize();
        switch(side) {
        case ELeftTop: {
            c.m_y  = 0;
            c.m_x  = 0;
        } break;
        case ERightTop: {
            c.m_y  = 0;
            c.m_x -= 1;
        } break;
        case ERightBottom: {
            c.m_y -= 1;
            c.m_x -= 1;
        } break;
        case ELeftBottom: {
            c.m_y -= 1;
            c.m_x  = 0;
        }
        }
        return c;
    }
    CCoord3D GetCornerCoord_abs(EPinSide side = ELeftTop) {
        return m_absoluteCoord + GetCornerCoord(side);
    }
    CPiece* GetSafe(CCoord2D p) {
        if (!ContainCoord(p)) return NULL;
        return Get(p);
    }
    CPiece*& Get(CCoord2D p) {
        return m_field[p.m_x][p.m_y];
    }
    CPiece*& Get(int x, int y) {
        return m_field[x][y];
    }
    CPiece*& Get_abs(CCoord2D p) {
        return Get(ToRelativeCoord(p));
    }
    CPiece*& Get_abs(CCoord3D p) {
        assert(p.m_level == m_absoluteCoord.m_level);
        return Get(ToRelativeCoord(p.Get2DPart()));
    }    
    CPiece* GetSafe_abs(CCoord2D p) {
        return GetSafe(ToRelativeCoord(p));
    }
    CPiece* GetSafe_abs(CCoord3D p) {
        if (!ContainCoord_abs(p)) return NULL;
        return Get(ToRelativeCoord(p.Get2DPart()));
    }
#ifdef USE_MPI
    bool CheckMove(CCoord3D fromAbs, CCoord3D toAbs, int rank) {
        LogEx("Check we have from coord " << fromAbs);
        if (NULL == GetSafe_abs(fromAbs)) return false;

        CCoord2D from = ToRelativeCoord(fromAbs);
        CCoord2D to = ToRelativeCoord(toAbs);

        LogEx("Build move path");
            
        T2DPath path = Get(from)->PlanMove(to - from);
        LogEx("Path: " << path);
        if (0 == path.size()) return false;
        
        LogEx("Check someone have to coord");
        if (!ContainCoord_abs(toAbs) && !MPI_ContainCoord_abs(rank, toAbs)) return false;
        LogEx("Check is toCoord on our board");
        if (ContainCoord_abs(toAbs)) {
            LogEx("Ok, toCoord on our board");
            LogEx("Check does to coord have not our pieces");
            if (NULL != Get(to) && Get(from)->m_color == Get(to)->m_color) {
                return false;
            }
            LogEx("Build move path");
            
            CCoord2D p = fromAbs.Get2DPart();
            for (T2DPath::iterator it = path.begin(); it != path.end(); ++it) {
                p = p + *it;
                LogEx("p: " << p);
                if (it != --path.end() && it->m_jump == false && NULL != GetSafe_abs(p)) {
                    LogEx("we have here piece");
                    return false;
                }
            }
            assert( toAbs.Get2DPart() == p );
            
            LogEx("Perform move");
            delete Get(to);
            Get(to) = Get(from);
            Get(from) = NULL;
        } else {
            LogEx("Check does to coord have not our pieces");
            EColor color;
            if ((color = MPI_CellColor(rank, toAbs)) == Get(from)->m_color) {
                LogEx("Got color " << colorToStr[color]);
                LogEx("Have color " << colorToStr[Get(from)->m_color]);            
                return false;
            }
            
            CCoord2D p = fromAbs.Get2DPart();
            for (T2DPath::iterator it = path.begin(); it != path.end(); ++it) {
                p = p + *it;
                LogEx("p: " << p);
                if (it != --path.end() && it->m_jump == false && 
                    (NULL != GetSafe_abs(p) || MPI_AnyCellNonEmpty_abs(rank, p)))
                {
                    LogEx("someone have here piece");
                    return false;
                }
            }
            assert( toAbs.Get2DPart() == p );
            
            LogEx("Perform move");
            assert( false != MPI_SetPiece(rank, toAbs, Get(from)->GetPieceTipe(), Get(from)->m_color));
            delete Get(from);
        }
        Get(from) = NULL;

        LogEx("Move is possible");
        return true;
    }
#else
    bool CheckMove(CCoord3D fromAbs, CCoord3D toAbs, CBoardBroadcast& broadcast) {
        LogEx("Check we have from coord " << fromAbs);
        if (NULL == GetSafe_abs(fromAbs)) return false;

        CCoord2D from = ToRelativeCoord(fromAbs);
        CCoord2D to = ToRelativeCoord(toAbs);

        LogEx("Check someone have to coord");
        if (!broadcast.ContainCoord_abs(toAbs)) return false;
        LogEx("Check does to coord have not our pieces");
        if (broadcast.CellColor(toAbs) == Get(from)->m_color) return false;
        LogEx("Build move path");
        
        T2DPath path = Get(from)->PlanMove(to - from);
        LogEx("Path: " << path);
        if (0 == path.size()) return false;
        
        CCoord2D p = fromAbs.Get2DPart();
        for (T2DPath::iterator it = path.begin(); it != path.end(); ++it) {
            p = p + *it;
            LogEx("p: " << p);
            if (it != --path.end() && it->m_jump == false && broadcast.AnyCellNonEmpty_abs(p)) {
                LogEx("someone have here piece");
                return false;
            }
        }
        assert( toAbs.Get2DPart() == p );

        return true;
    }
#endif
    CCoord2D ToRelativeCoord(CCoord2D c) { return c - m_absoluteCoord.Get2DPart(); }
    CCoord2D ToRelativeCoord(CCoord3D c) { return c.Get2DPart() - m_absoluteCoord.Get2DPart(); }
    EColor GetFigureColor_abs(CCoord2D absCoord) {
        CCoord2D relCoord = absCoord - m_absoluteCoord.Get2DPart();
        if (NULL == Get(relCoord)) return ENone;
        return Get(relCoord)->m_color;
    }
    EColor GetFigureColor_abs(CCoord3D absCoord) {
        if (absCoord.m_level != m_level) return ENone;
        return GetFigureColor_abs(absCoord.Get2DPart());
    }
    void Dump(std::ostream& out, int offset = 0) {
        int fstLineNum = m_absoluteCoord.m_y;
        CCoord2D size = GetSize();
        for (int y = 0; y < size.m_y; ++y) {
            out << fstLineNum++ << ' ';
            printSpaces(out, offset);
            for (int x = 0; x < size.m_x; ++x) {
                if (NULL == Get(CCoord2D(x, y))) {
                    out << '.';
                } else {
                    out << *Get(CCoord2D(x, y));
                }
            }
            if (y == 0) out << "   " << m_absoluteCoord.m_level << "\n";
            else out << "  \n";
        }
    }
};

inline std::ostream& operator<<(std::ostream& out, CBoard& board) {
    board.Dump(out, 0);
    return out;
}

#ifdef USE_MPI
struct CMainBoard: public CBoard {
    CMainBoard(IHaveRank& owner, ILogger& log): CBoard(owner, log) {}
#else
    struct CMainBoard: public CBoard {
#endif
    virtual CCoord2D GetSize() { return CCoord2D(4, 4); }
};

#ifdef USE_MPI
struct CAttackBoard: public CBoard {
    CAttackBoard(IHaveRank& owner, ILogger& log): CBoard(owner, log) {}
#else
    struct CAttackBoard: public CBoard {
#endif
    EColor m_color;
    virtual CCoord2D GetSize() { return CCoord2D(2, 2); }
    virtual EColor GetColor() { return m_color; }
    void AttachToPin(EPinSide side, CCoord3D pinCoord) {
        m_absoluteCoord = pinCoord;
        m_absoluteCoord.m_level += 1;
        switch(side) {
        case ELeftTop: {
            m_absoluteCoord.m_x -= 1;
            m_absoluteCoord.m_y -= 1;
        } break;
        case ERightTop: {
            m_absoluteCoord.m_y -= 1;
        } break;
        case ELeftBottom: {
            m_absoluteCoord.m_x -= 1;
        } break;
        case ERightBottom: {
        }
        }
    }
};

struct CBoardConfigure {
    static void SetWhiteFigures(CMainBoard& m, CAttackBoard& a1, CAttackBoard& a2) {
        SetWhiteMainFigures(m);
        SetWhiteAttack1Figures(a1);
        SetWhiteAttack2Figures(a2);
    }
    static void SetWhiteMainFigures(CMainBoard& m) {
        m.Get(0, 1) = new CPawn;
        m.Get(1, 1) = new CPawn;
        m.Get(2, 1) = new CPawn;
        m.Get(3, 1) = new CPawn;
        m.Get(0, 0) = new CKnight;
        m.Get(1, 0) = new CBishop;
        m.Get(2, 0) = new CBishop;
        m.Get(3, 0) = new CKnight;
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 4; ++x)
                m.Get(x, y)->m_color = EWhite;
    }
    static void SetWhiteAttack1Figures(CAttackBoard& a1) {
        a1.m_color = EWhite;
        a1.Get(0, 0) = new CRook;
        a1.Get(1, 0) = new CQueen;
        a1.Get(0, 1) = new CPawn;
        a1.Get(1, 1) = new CPawn;
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a1.Get(x, y)->m_color = EWhite;
    }
    static void SetWhiteAttack2Figures(CAttackBoard& a2) {
        a2.m_color = EWhite;
        a2.Get(0, 0) = new CKing;
        a2.Get(1, 0) = new CRook;
        a2.Get(0, 1) = new CPawn;
        a2.Get(1, 1) = new CPawn;        
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a2.Get(x, y)->m_color = EWhite;
    }
    static void SetBlackFigures(CMainBoard& m, CAttackBoard& a1, CAttackBoard& a2) {
        SetBlackMainFigures(m);
        SetBlackAttack1Figures(a1);
        SetBlackAttack2Figures(a2);
    }
    static void SetBlackMainFigures(CMainBoard& m) {
        m.Get(0, 2) = new CPawn;
        m.Get(1, 2) = new CPawn;
        m.Get(2, 2) = new CPawn;
        m.Get(3, 2) = new CPawn;
        m.Get(0, 3) = new CKnight;
        m.Get(1, 3) = new CBishop;
        m.Get(2, 3) = new CBishop;
        m.Get(3, 3) = new CKnight;
        for (int y = 2; y < 4; ++y)
            for (int x = 0; x < 4; ++x)
                m.Get(x, y)->m_color = EBlack;
    }
    static void SetBlackAttack1Figures(CAttackBoard& a1) {
        a1.m_color = EBlack;
        a1.Get(0, 1) = new CRook;
        a1.Get(1, 1) = new CQueen;
        a1.Get(0, 0) = new CPawn;
        a1.Get(1, 0) = new CPawn;
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a1.Get(x, y)->m_color = EBlack;
    }
    static void SetBlackAttack2Figures(CAttackBoard& a2) {
        a2.m_color = EBlack;
        a2.Get(0, 1) = new CKing;
        a2.Get(1, 1) = new CRook;
        a2.Get(0, 0) = new CPawn;
        a2.Get(1, 0) = new CPawn;           
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a2.Get(x, y)->m_color = EBlack;
    }
    static CCoord3D GetInitialCoords(int rank) {
        if (rank == 1) return CCoord3D(1, 1, 1);
        if (rank == 2) return CCoord3D(1, 3, 3);
        if (rank == 3) return CCoord3D(1, 5, 5);
        if (rank == 4) return CCoord3D(0, 0, 2);
        if (rank == 5) return CCoord3D(4, 0, 2);
        if (rank == 6) return CCoord3D(0, 8, 6);
        if (rank == 7) return CCoord3D(4, 8, 6);        
        assert(0);
    }
};

#endif
