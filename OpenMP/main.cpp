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
    case EInsert: {
        arr.InsertSort();
    } break;
    default: {
        throw("Bad thing");
    }
    }
    gettimeofday(&tv2, NULL);
    return diff_to_sec(&tv1, &tv2);
}

double RunManyTimes(unsigned runs, unsigned arrSize, ESortMethod sort) {
    CSimpleArray arr(arrSize);
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

int main()
{
    for (unsigned i = 0; i < 3; ++i) {
        std::ofstream fquick, fmerge;
        openFiles(fquick, fmerge, i);
        
        unsigned arrSize = sizes[i];
        unsigned repeatCnt = runs[i];
        std::cerr << "Array size: " << arrSize;
        for (int tnum = 1; tnum < 30; ++ tnum) {
            omp_set_num_threads(tnum);
            std::cerr << "*";
            fmerge << tnum << " " << RunManyTimes(repeatCnt, arrSize, EMerge) << "\n";
            fquick << tnum << " " << RunManyTimes(repeatCnt, arrSize, EQuick) << "\n";
        }
        std::cerr << "\n";
        fquick.close();
        fmerge.close();
    }
    return 0;
}
