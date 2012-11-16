#ifndef _CHESS_H
#define _CHESS_H

#include <cassert>
#include <vector>
#include <iostream>

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
    int m_dx;
    int m_dy;
    bool m_jump;
};

typedef std::vector<CMove> TPath;

struct CPiece {
    CCoord2D m_relativeCoords;
    virtual EPieces GetPieceTipe() = 0;
    virtual TPath PlanMove(CCoord2D relativeCoords) = 0;
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
