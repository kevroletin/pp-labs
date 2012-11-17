#include "mpi_helpers.h"
#include "chess.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

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
