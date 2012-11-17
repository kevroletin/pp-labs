#ifndef _CHESS_H
#define _CHESS_H

#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

#undef MPI
//#define MPI

#if 0
#  define Log(str) std::cerr << str << "\n";
#else
#  define Log(str)
#endif


inline int sign(int x) { return x > 0 ? 1 : x == 0 ? 0 : -1; }
inline void printSpaces(std::ostream& out, int x) { while(x--) { out << ' '; } }

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

std::ostream& operator<<(std::ostream& out, CCoord2D c) {
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

std::ostream& operator<<(std::ostream& out, CCoord3D c) {
    out << "x:" << c.m_x << " y:" << c.m_y << " level:" << c.m_level;
    return out;
}

enum EColor {
    EWhite = 1,
    ENone  = 0,
    EBlack = -1,
};

std::string colorToStr[] = { "white", "none", "black" };

enum ESide {
    ETop,
    ERight,
    EBottom,
    ELeft,
};

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
    ERook    //Ладья
};

std::string piecesToStr[] = { "K", "P", "Q", "K", "B", "R" };

struct CMove {
    CMove(int dx, int dy, bool jump = false): m_dx(dx), m_dy(dy), m_jump(jump) {}
    CMove(CCoord2D c, bool jump = false): m_dx(c.m_x), m_dy(c.m_y), m_jump(jump) {}
    int m_dx;
    int m_dy;
    bool m_jump;
};

CCoord2D operator+(CCoord2D c, CMove m) {
    c.m_x += m.m_dx;
    c.m_y += m.m_dy;
    return c;
}

CCoord3D operator+(CCoord3D c, CMove m) {
    c.m_x += m.m_dx;
    c.m_y += m.m_dy;
    return c;
}

std::ostream& operator<<(std::ostream& out, CMove move) {
    if (move.m_jump) out << "*";    
    out << "dx:" << move.m_dx << " dy:" << move.m_dy;
    return out;
}

typedef std::vector<CMove> T2DPath;
typedef std::vector<CCoord3D> T3DPath;

std::ostream& operator<<(std::ostream& out, const T2DPath& path) {
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

std::ostream& operator<<(std::ostream& out, CPiece& p) {
    std::string str = piecesToStr[p.GetPieceTipe()];
    if (EBlack == p.m_color) {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
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
        if (c.m_x != 0) return path;
        if (c.m_y == m_color) {
            path.push_back(c);
        } else if (m_firstMove && c.m_y == 2*m_color) {
            path.push_back(CMove(0, m_color));
            path.push_back(CMove(0, m_color));
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

struct CBoard {
    static const int maxBoardSize = 4;

    int m_level;
    CCoord3D m_absoluteCoord;
    CPiece* m_field[maxBoardSize][maxBoardSize];
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
    
#ifdef MPI
#  error "Not implemented"
#else
    bool CheckMove(CCoord3D fromAbs, CCoord3D toAbs, CBoardBroadcast& broadcast) {
        Log("Check we have from coord " << fromAbs);
//        if (!ContainCoord_abs(fromAbs)) return false;
        if (NULL == GetSafe_abs(fromAbs)) return false;

        CCoord2D from = ToRelativeCoord(fromAbs);
        CCoord2D to = ToRelativeCoord(toAbs);

//        Log("Check we own piece on from cord");
        Log("Check someone have to coord");
        if (!broadcast.ContainCoord_abs(toAbs)) return false;
        Log("Check does to coord have not our pieces");
        if (broadcast.CellColor(toAbs) == Get(from)->m_color && Get(from)->m_color != ENone) return false;
        Log("Build move path");
        
        T2DPath path = Get(from)->PlanMove(to - from);
        Log("Path: " << path);
        if (0 == path.size()) return false;
        
        CCoord2D p = fromAbs.Get2DPart();
        for (T2DPath::iterator it = path.begin(); it != path.end(); ++it) {
            p = p + *it;
            Log("p: " << p);
            if (it != --path.end() && it->m_jump == false && broadcast.AnyCellNonEmpty_abs(p)) {
                Log("someone have here piece");
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
            else out << "\n";
        }
    }
};

struct CMainBoard: public CBoard {
    virtual CCoord2D GetSize() { return CCoord2D(4, 4); }
};

struct CAttackBoard: public CBoard {
    EColor m_color;
    virtual CCoord2D GetSize() { return CCoord2D(2, 2); }
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

struct CSimpleBroadcast: public CBoardBroadcast {
    CMainBoard m_mainBoards[3];
    CAttackBoard m_attackBoards[4];
    CBoard* GetBoard(CCoord3D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (m_mainBoards[i].ContainCoord_abs(absCoord)) {
                return &m_mainBoards[i];
            }
        }
        for (int i = 0; i < 4; ++i) {
            if (m_attackBoards[i].ContainCoord_abs(absCoord)) {
                return &m_attackBoards[i];
            }
        }        
        return NULL;
    }
    CMainBoard* GetMainBoard(CCoord3D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (m_mainBoards[i].ContainCoord_abs(absCoord)) {
                return &m_mainBoards[i];
            }
        }
        return NULL;
    }
    CAttackBoard* GetAttackBoard(CCoord3D absCoord) {
        for (int i = 0; i < 4; ++i) {
            if (m_attackBoards[i].ContainCoord_abs(absCoord)) {
                return &m_attackBoards[i];
            }
        }
        return NULL;
    }
    virtual EColor CellColor(CCoord3D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (m_mainBoards[i].ContainCoord_abs(absCoord)) {
                CPiece* p = m_mainBoards[i].GetSafe_abs(absCoord);
                if (NULL != p) return p->m_color;
            }
        }
        for (int i = 0; i < 4; ++i) {
            if (m_attackBoards[i].ContainCoord_abs(absCoord)) {
                CPiece* p = m_attackBoards[i].GetSafe_abs(absCoord);
                if (NULL != p) return p->m_color;
            }
        }        
        return ENone;
    }
    bool Move(CCoord3D fromAbs, CCoord3D toAbs) {
        CBoard* bf = GetBoard(fromAbs);
        CBoard* bt = GetBoard(toAbs);
        if (NULL == bf || NULL == bt) return NULL;
        if (!bf->CheckMove(fromAbs, toAbs, *this)) return false;
        delete bt->Get_abs(toAbs);
        bt->Get_abs(toAbs) = bf->Get_abs(fromAbs);
        bf->Get_abs(fromAbs) = NULL;
        return true;
    }
    bool MoveBoard(CCoord3D fromAbs, CCoord3D toAbs) {
        Log("Check allowed attack board level " << toAbs.m_level);
        if (toAbs.m_level != 2 && toAbs.m_level != 4 && toAbs.m_level != 6) return false;
        Log("Find attack board");
        CAttackBoard* b = GetAttackBoard(fromAbs);
        if (NULL == b) return false;
        Log("Check if new coords are free");
        if (NULL != GetAttackBoard(toAbs)) return false;
        Log("Update coords");
        // TODO: add proper cheks
        b->m_absoluteCoord = toAbs;
        return true;
    }
    CSimpleBroadcast() {
        for (int i = 0; i < 3; ++i) m_mainBoards[i].Init();
        for (int i = 0; i < 4; ++i) m_attackBoards[i].Init();
        SetWhiteFigures(m_mainBoards[0], m_attackBoards[0], m_attackBoards[1]);
        SetBlackFigures(m_mainBoards[2], m_attackBoards[2], m_attackBoards[3]);
        m_mainBoards[0].m_absoluteCoord = CCoord3D(1, 1, 1);
        m_mainBoards[1].m_absoluteCoord = CCoord3D(1, 3, 3);
        m_mainBoards[2].m_absoluteCoord = CCoord3D(1, 5, 5);

        m_attackBoards[0].AttachToPin(ELeftTop, m_mainBoards[0].GetCornerCoord_abs(ELeftTop));
        m_attackBoards[1].AttachToPin(ERightTop, m_mainBoards[0].GetCornerCoord_abs(ERightTop));
        m_attackBoards[2].AttachToPin(ELeftBottom, m_mainBoards[2].GetCornerCoord_abs(ELeftBottom));
        m_attackBoards[3].AttachToPin(ERightBottom, m_mainBoards[2].GetCornerCoord_abs(ERightBottom));
    }
    void SetWhiteFigures(CMainBoard& m, CAttackBoard& a1, CAttackBoard& a2) {
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
        a1.m_color = a2.m_color = EWhite;
        a1.Get(0, 0) = new CRook;
        a1.Get(1, 0) = new CQueen;
        a1.Get(0, 1) = new CPawn;
        a1.Get(1, 1) = new CPawn;
        a2.Get(0, 0) = new CKing;
        a2.Get(1, 0) = new CRook;
        a2.Get(0, 1) = new CPawn;
        a2.Get(1, 1) = new CPawn;        
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a1.Get(x, y)->m_color = a2.Get(x, y)->m_color = EWhite;
    }
    void SetBlackFigures(CMainBoard& m, CAttackBoard& a1, CAttackBoard& a2) {
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
        a1.m_color = a2.m_color = EBlack;
        a1.Get(0, 1) = new CRook;
        a1.Get(1, 1) = new CQueen;
        a1.Get(0, 0) = new CPawn;
        a1.Get(1, 0) = new CPawn;
        a2.Get(0, 1) = new CKing;
        a2.Get(1, 1) = new CRook;
        a2.Get(0, 0) = new CPawn;
        a2.Get(1, 0) = new CPawn;        
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 2; ++x)
                a1.Get(x, y)->m_color = a2.Get(x, y)->m_color = EBlack;
    }
    virtual bool AnyCellNonEmpty_abs(CCoord2D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (NULL != m_mainBoards[i].GetSafe(absCoord)) return true;
        }
        for (int i = 0; i < 4; ++i) {
            if (NULL != m_attackBoards[i].GetSafe(absCoord)) return true;
        }
        return false;
    }
    virtual EColor CellColor_abs(CCoord3D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (NULL != m_mainBoards[i].GetSafe_abs(absCoord)) {
                return m_mainBoards[i].GetSafe_abs(absCoord)->m_color;
            }
        }
        for (int i = 0; i < 4; ++i) {
            if (NULL != m_attackBoards[i].GetSafe_abs(absCoord)) {
                return m_attackBoards[i].GetSafe_abs(absCoord)->m_color;
            }
        }
        return ENone;
    }
    virtual bool ContainCoord_abs(CCoord3D absCoord) {
        for (int i = 0; i < 3; ++i) {
            if (m_mainBoards[i].ContainCoord_abs(absCoord)) return true;
        }
        for (int i = 0; i < 4; ++i) {
            if (m_attackBoards[i].ContainCoord_abs(absCoord)) return true;
        }
        return ENone;        
    }
    void Dump(std::ostream& out) {
        for (int i = 0; i < 3; ++i) {
            out << "main board#" << i << ": " << m_mainBoards[i].m_absoluteCoord << "\n";
        }
        for (int i = 0; i < 4; ++i) {
            out << "attack board#" << i << ": " << m_attackBoards[i].m_absoluteCoord << " " <<
                colorToStr[m_attackBoards[i].m_color + 1] << "\n";
        }
        out << "  ";
        for (int i = 0; i < 6; ++i) {
            out << i;
            if (i == 1 || i == 4) out << i;
        }
        out << "\n";
        DumpAttackBoardsLine(out, 0, 2);
        m_mainBoards[0].Dump(out, 2);
        DumpAttackBoardsLine(out, 4, 2);

        DumpAttackBoardsLine(out, 2, 4);
        m_mainBoards[1].Dump(out, 2);
        DumpAttackBoardsLine(out, 6, 4);
        
        DumpAttackBoardsLine(out, 4, 6);
        m_mainBoards[2].Dump(out, 2);
        DumpAttackBoardsLine(out, 8, 6);
    }
    void DumpAttackBoardsLine(std::ostream& out, int startY, int level) {
        for (int y = startY; y < startY + 2; ++y) {
            out << y << ' ';
            for (int x = 0; x < 6; ++x) {
                bool inside = false;
                bool printed = false;
                
                for (int i = 0; i < 4 && !printed; ++i) {
                    if (m_attackBoards[i].m_absoluteCoord.m_level != level) continue;
                    CCoord2D p = CCoord2D(x, y);
                    if (m_attackBoards[i].ContainCoord_abs(p)) {
                        inside = true;
                        if (NULL != m_attackBoards[i].GetSafe_abs(p)) {
                            out << *m_attackBoards[i].GetSafe_abs(p);
                            printed = true;
                        }
                    }
                }
                if (!printed) {
                    out << (inside ? '.' : ' ');
                }
                if (x == 2) {
                    out << "  ";
                }
            }
            if (y == startY) out << " " << level << "\n";
            else out << "\n";
        }
    }
};

#endif
