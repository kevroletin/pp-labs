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
    baseName("chess");
    {
        name("coordinates == !=");
        ok( CCoord2D(1, 2) == CCoord2D(1, 2) );
        ok( CCoord2D(1, 2) != CCoord2D(2, 1) );
        ok( CCoord3D(1, 2, 3) == CCoord3D(1, 2, 3) );
        ok( CCoord3D(1, 2, 4) != CCoord3D(1, 2, 3) );
        ok( CCoord3D(2, 1, 3) != CCoord3D(1, 2, 3) );
        name("coordinates + -");
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

    return 0;
}
