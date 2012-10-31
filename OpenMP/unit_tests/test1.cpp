#include "../array.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

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

enum ESortMethod {
    EMerge,
    EQuick,
    EInsert,
    LastSortMethod
};

std::string sortMethodStr[] = {
    "MergeSort",
    "QuickSort",
    "InsertSort",
    "LastSortMethod"
};

void RunSort(CSimpleArray& arr, ESortMethod sort) {
    switch (sort) {
    case EMerge: {
        arr.MergeSort();
    } break;
    case EQuick: {
        arr.QuickSort();
    }break;
    case EInsert: {
        arr.InsertSort();
    } break;
    default: {
        throw("Bad thing");
    }
    }
}

unsigned sizes[][6] = {
  { 10, 100, 500, 1000, 50000, 10000 },
  { 10, 100, 500, 1000, 50000, 10000 },
  { 10, 100, 500, 1000, 0, 0 }
};

int main() {
    baseName("init");
    name("check is OpenMP enabled");
#ifdef _OPENMP
    omp_set_num_threads(2);
    ok(1);
#else
#   warning "OpenMP disabled"
    ok(0);
#endif
    srand(time(NULL));
    for (unsigned k = 0; k < 3; ++k) {
        ESortFinOptimization sortOpt = static_cast<ESortFinOptimization>(k);
        for (int j = 0; j < LastSortMethod; ++j) {
            baseName(sortMethodStr[j]);
            ESortMethod sortAlg = static_cast<ESortMethod>(j);
        
            name("Self check");
            for (unsigned i = 0; i < 6; ++i) {
                unsigned size = sizes[j][i];
                if (size == 0) continue;
                {
                    CSimpleArray arr(size, sortOpt);
                    arr.FillDec();
                    RunSort( arr, sortAlg );
                    ok( arr.ValidateSort() );
                }
                {
                    CSimpleArray arr(size, sortOpt);
                    arr.FillInc();
                    RunSort( arr, sortAlg );
                    ok( arr.ValidateSort() );
                }    
                {
                    CSimpleArray arr(size, sortOpt);
                    arr.FillRand();
                    RunSort( arr, sortAlg );
                    ok( arr.ValidateSort() );
                }
            }
            
            name("compare with vector");
            for (unsigned i = 0; i < 6; ++i) {
                unsigned size = sizes[j][i];
                if (size > 0) {
                    CSimpleArray arr(size, sortOpt);
                    std::vector<TData> vec(size);
                    for (unsigned i = 0; i < size; ++i) {
                    arr[i] = vec[i] = rand();
                    }
                    sort(vec.begin(), vec.end());
                    RunSort( arr, sortAlg );
                    bool same = true;
                    for (unsigned i = 0; i < size; ++i) {
                        same &= vec[i] == arr[i];
                    }
                    ok( same );
                }
            }
        }
    }
        
    if (executed == ok_cnt) {
        std::cerr << "=== pass ===\n";
    } else {
        std::cerr << "=== fail ===\n";
    }

    return 0;
}
