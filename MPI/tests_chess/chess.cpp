#include "mpi_helpers.h"
#include "chess.h"
#include <iostream>
#include <cassert>
#include <vector>

int main()
{
    CSimpleBroadcast bc;
//    bc.Dump(std::cerr);

//    std::cerr << bc.m_mainBoards[0].CheckMove(, bc) << "\n";
    std::cerr << bc.Move(CCoord3D(1, 2, 1), CCoord3D(1, 4, 3)) << "\n";
    bc.Dump(std::cerr);
    std::cerr << bc.Move(CCoord3D(1, 4, 3), CCoord3D(1, 5, 3)) << "\n";
    bc.Dump(std::cerr);
    
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
