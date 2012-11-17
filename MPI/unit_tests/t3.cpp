#include "../chess.h"
#include <sstream>

int executed = 0;
int ok_cnt = 0;
std::string currTestBatch = "";
std::string currTestName = "";

void baseName(std::string name) { currTestBatch = name; }
void baseName() { currTestBatch = ""; };
void name(std::string name) { currTestName = name; }
void name() { currTestName = ""; };

void ok(bool res) {
    ++executed;
    std::stringstream ss;
    ss << executed << " " << currTestBatch << ":" << currTestName << " - " ;
    if (res) {
        ++ok_cnt;
        ss << "ok";
    } else {
        ss << "fail";
    }
    ss << "\n";
    std::cerr << ss.str();
}

int main()
{
    baseName("coordinates");
    {
        name("== !=");
        ok( CCoord2D(1, 2) == CCoord2D(1, 2) );
        ok( CCoord2D(1, 2) != CCoord2D(2, 1) );
        ok( CCoord3D(1, 2, 3) == CCoord3D(1, 2, 3) );
        ok( CCoord3D(1, 2, 4) != CCoord3D(1, 2, 3) );
        ok( CCoord3D(2, 1, 3) != CCoord3D(1, 2, 3) );
        name("+ -");
        ok( CCoord2D(1, 2) + CCoord2D(1, 1) == CCoord2D(2, 3) );
        ok( CCoord2D(1, 2) - CCoord2D(1, 1) == CCoord2D(0, 1) );
        ok( CCoord3D(1, 2, 5) + CCoord2D(1, 1) == CCoord3D(2, 3, 5) );
        ok( CCoord3D(1, 2, 5) - CCoord2D(1, 1) == CCoord3D(0, 1, 5) );
        ok( CCoord3D(1, 2, 5) + CCoord3D(1, 1, 1) == CCoord3D(2, 3, 6) );
        ok( CCoord3D(1, 2, 5) - CCoord3D(1, 1, 1) == CCoord3D(0, 1, 4) );
        name("inside");
        ok( CCoord2D(2, 2).Inside( CCoord2D(1, 1) ) );
        ok( !CCoord2D(2, 2).Inside( CCoord2D(2, 2) ) );
        name("get 2d pard");
        ok( CCoord3D(1, 2, 3).Get2DPart() == CCoord2D(1, 2) );
    }
    baseName("boards");
    {
        CMainBoard b;
        b.m_absoluteCoord = CCoord3D(1, 3, 3);

        name("size");
        ok(b.GetSize() == CCoord2D(4, 4));

        name("relative coords");
        ok(b.ContainCoord(CCoord2D(0, 0)));
        {
            bool good = true;
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    good &= b.ContainCoord(CCoord2D(x, y));
                }
            }
            ok(good);
        }
        {
            bool good = true;
            for (int i = -1; i <= 4; ++i) {
                good &= !b.ContainCoord(CCoord2D(i, -1));
                good &= !b.ContainCoord(CCoord2D(i, 4));
                good &= !b.ContainCoord(CCoord2D(-1, i));
                good &= !b.ContainCoord(CCoord2D(4, i));
            }
            ok(good);
        }

        name("abs coords");
        ok(b.ContainCoord_abs(CCoord3D(1, 3, 3)));
        CCoord3D p = CCoord3D(1, 3, 1);
        assert(p.m_level == 1 && b.m_absoluteCoord.m_level == 3);
        ok(!b.ContainCoord_abs(p));
        ok(!b.ContainCoord_abs(CCoord3D(1, 1, 3)));

        name("corners");
        ok(b.GetCornerCoord(ELeftTop) == CCoord2D(0, 0));
        ok(b.GetCornerCoord(ERightTop) == CCoord2D(3, 0));
        ok(b.GetCornerCoord(ELeftBottom) == CCoord2D(0, 3));
        ok(b.GetCornerCoord(ERightBottom) == CCoord2D(3, 3));

        name("abs corners");
        ok(b.GetCornerCoord_abs(ELeftTop) == CCoord3D(1, 3, 3));
        ok(b.GetCornerCoord_abs(ERightTop) == CCoord3D(4, 3, 3));
        ok(b.GetCornerCoord_abs(ELeftBottom) == CCoord3D(1, 6, 3));
        ok(b.GetCornerCoord_abs(ERightBottom) == CCoord3D(4, 6, 3));
    }
    baseName("attack boards");
    {
        CMainBoard b;
        b.m_absoluteCoord = CCoord3D(1, 3, 3);
        CAttackBoard a;
        a.AttachToPin(ELeftTop, b.GetCornerCoord_abs(ELeftTop));
        ok( a.m_absoluteCoord == CCoord3D(0, 2, 4) );
        a.AttachToPin(ERightTop, b.GetCornerCoord_abs(ERightTop));
        ok( a.m_absoluteCoord == CCoord3D(4, 2, 4) );
        a.AttachToPin(ELeftBottom, b.GetCornerCoord_abs(ELeftBottom));
        ok( a.m_absoluteCoord == CCoord3D(0, 6, 4) );
        a.AttachToPin(ERightBottom, b.GetCornerCoord_abs(ERightBottom));
        ok( a.m_absoluteCoord == CCoord3D(4, 6, 4) );
    }
    return 0;
}
