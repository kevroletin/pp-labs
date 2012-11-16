#ifndef _CHESS_H
#define _CHESS_H

#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

inline int sign(int x) { return x > 0 ? 1 : x == 0 ? 0 : -1; }

struct CCoord2D {
    CCoord2D(int x = 0, int y = 0): m_x(x), m_y(y) {}
    bool Inside(CCoord2D p) {
        assert(p.m_x >= 0 && p.m_y >= 0);
        return p.m_x < m_x && p.m_y < m_y;
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
    EBlack = -1,
};

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

struct CMove {
    CMove(int dx, int dy, bool jump = false): m_dx(dx), m_dy(dy), m_jump(jump) {}
    CMove(CCoord2D c, bool jump = false): m_dx(c.m_x), m_dy(c.m_y), m_jump(jump) {}
    int m_dx;
    int m_dy;
    bool m_jump;
};

std::ostream& operator<<(std::ostream& out, CMove move) {
    if (move.m_jump) out << "*";    
    out << "dx:" << move.m_dx << " dy:" << move.m_dy;
    return out;
}

typedef std::vector<CMove> TPath;

std::ostream& operator<<(std::ostream& out, const TPath& path) {
    TPath::const_iterator it = path.begin();
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
    virtual TPath PlanMove(CCoord2D relativeCoords) = 0;
    virtual void BeforeMove() {}
};

struct CKnight: public CPiece {
    virtual EPieces GetPieceTipe() { return EKnight; }
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
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
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
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
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
        if (abs(c.m_x) == 1 && abs(c.m_y) <= 1) {
            path.push_back(c);
        }
        return path;
    }
};

struct CBishop: public CPiece {
    virtual EPieces GetPieceTipe() { return EBishop; }
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
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
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
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
    virtual TPath PlanMove(CCoord2D c) {
        TPath path;
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

struct IBoard {
    virtual CCoord2D GetSize() = 0;
    virtual CCoord3D GetCornerCoord(EPinSide side = ELeftTop) = 0;
    bool ContainCoord(CCoord3D coord) {
        CCoord2D size = GetSize();
        CCoord3D borderCoord = GetCornerCoord();
        return borderCoord.m_level == coord.m_level &&
               size.Inside((coord - borderCoord).Get2DPart());
    }
    bool ContainCoord(CCoord2D coord) {
        CCoord2D size = GetSize();
        CCoord3D borderCoord = GetCornerCoord();
        return size.Inside(coord - borderCoord.Get2DPart());
    }
    
};

#endif
