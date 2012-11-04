#include <iostream>
#include <fstream>
#include "../mpi_helpers.h"
#include "../labyrinth.h"


void test(std::string fname)
{
    std::ifstream fin;
    std::string ans;
    fin.open(fname.c_str());
    
    MixTaskLogger* log = new MixTaskLogger(0);
    CFieldReder r(*log);
    r.Read(fin);
    fin  >> ans;
//    r.Dump(std::cerr);
    std::string way;
    if (!r.FindWay(way)) {
        way = "fail";
    }
//    r.Dump(std::cerr);
//    r.Dump(std::cerr, false);    
//    log->PublishLog(std::cerr);
    if (ans == way) {
        std::cerr << "ok\n";
    } else {
        std::cerr << "fail\n";
    }
    fin.close();
}

int main()
{
    test("t1/01_in.txt");
    test("t1/02_in.txt");
    test("t1/03_in.txt");
    test("t1/04_in.txt");
    test("t1/05_in.txt");
    test("t1/06_in.txt");

    return 0;
}
