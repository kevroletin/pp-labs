#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

#undef Log
#undef LogEx
#define Log(str)
#define LogEx(str)
//#define Log(str) std::cerr << str << "\n";
//#define LogEx(str) std::cerr << str << "\n";

#include "chess.h"

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
        LogEx("Check allowed attack board level " << toAbs.m_level);
        if (toAbs.m_level != 2 && toAbs.m_level != 4 && toAbs.m_level != 6) return false;
        LogEx("Find attack board");
        CAttackBoard* b = GetAttackBoard(fromAbs);
        if (NULL == b) return false;
        LogEx("Check if new coords are free");
        if (NULL != GetAttackBoard(toAbs)) return false;
        LogEx("Update coords");
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
            if (NULL != m_mainBoards[i].GetSafe_abs(absCoord)) return true;
        }
        for (int i = 0; i < 4; ++i) {
            if (NULL != m_attackBoards[i].GetSafe_abs(absCoord)) return true;
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
                colorToStr[m_attackBoards[i].m_color] << "\n";
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

int main(int argc, char* argv[])
{
    std::ifstream in;
    std::ofstream out;
    assert(argc == 3);
    in.open(argv[1]);
    out.open(argv[2]);
    {
        CSimpleBroadcast bc;
        char c;
        int xf, yf, levelf;
        int xt, yt, levelt; 
        while( in >> c >> xf >> yf >> levelf >> xt >> yt >> levelt ) {
            if (c == 'M') {
                bc.Move(CCoord3D(xf, yf, levelf), CCoord3D(xt, yt, levelt));
            } else {
                bc.MoveBoard(CCoord3D(xf, yf, levelf), CCoord3D(xt, yt, levelt));
            }
            bc.Dump(out);
        }
    }
    in.close();
    out.close();
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
