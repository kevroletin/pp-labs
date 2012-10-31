#include "array.h"
#include <fstream>
#include <omp.h>
#include <sys/time.h>

#ifndef _OPENMP
#   warning "OpenMP disabled. For gcc add -fopenmp switch"
#endif

double diff_to_sec(struct timeval *a, struct timeval *b)
{
    return b->tv_sec - a->tv_sec + 10e-7 * (b->tv_usec - a->tv_usec);
}

enum ESortMethod {
    EMerge,
    EQuick,
    EInsert,
    LastSortMethod
};

double RunSort(CSimpleArray& arr, ESortMethod sort) {
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    switch (sort) {
    case EMerge: {
        arr.MergeSort();
    } break;
    case EQuick: {
        arr.QuickSort();
    }break;
    default: {
        throw("Bad thing");
    }
    }
    gettimeofday(&tv2, NULL);
    return diff_to_sec(&tv1, &tv2);
}

double RunManyTimes(CSimpleArray& arr, unsigned runs, ESortMethod sort) {
    arr.FillRand();
    double result = 0;
    for (unsigned i = 0; i < runs; ++i) {
        std::cerr << ".";
        result += RunSort(arr, sort);
    }
    return result / runs;
}

void openFiles(std::ofstream& fq, std::ofstream& fm, unsigned num) {
    char buff[100];
    sprintf(buff, "quick_%02d.res", num);
    fq.open(buff);
    sprintf(buff, "merge_%02d.res", num);
    fm.open(buff);
}

unsigned sizes[] = { 1000, 10000, 10000000 };
unsigned runs[]  = { 500,  10,    2 };

void DetermineBestThreadsNum() {
    for (unsigned i = 0; i < 3; ++i) {
        std::ofstream fquick, fmerge;
        openFiles(fquick, fmerge, i);
        
        unsigned arrSize = sizes[i];
        unsigned repeatCnt = runs[i];
        std::cerr << "Array size: " << arrSize;
        for (int tnum = 1; tnum < 30; ++ tnum) {
            CSimpleArray arr(arrSize);
            omp_set_num_threads(tnum);
            std::cerr << "*";
            fmerge << tnum << " " << RunManyTimes(arr, repeatCnt, EMerge) << "\n";
            fquick << tnum << " " << RunManyTimes(arr, repeatCnt, EQuick) << "\n";
        }
        std::cerr << "\n";
        fquick.close();
        fmerge.close();
    }
}

std::string optimizationToStr[] = {
    "Insert",
    "Qsort",
    "Merge"
};

void DetermineBestK() {
    for (unsigned i = 0; i < 3; ++i) {
        ESortFinOptimization sortOpt = static_cast<ESortFinOptimization>(i);
        std::ofstream fquick, fmerge;
        openFiles(fquick, fmerge, i);
        for (int k = 1; k < 50; k += 5) {
            CSimpleArray arr(10000000, k, sortOpt);
            unsigned repeatCnt = 1;
            std::cerr << "K: " << k;
            omp_set_num_threads(4);
            fquick << k << " " << RunManyTimes(arr, repeatCnt, EQuick) << "\n";
            fmerge << k << " " << RunManyTimes(arr, repeatCnt, EMerge) << "\n";
            std::cerr << "\n";
        }
        fquick.close();
        fmerge.close();
    }
}

int main()
{
    DetermineBestK();
    // TODO: add interface to choose
    //DetermineBestThreadsNum()
    return 0;
}

