#include <iostream>
#include <cassert>
#include <sstream>
#include <cstdlib>
#include <omp.h>

typedef unsigned TData;

#define LOG(str)
//#define LOG(str) std::cerr << str << "\n";

enum ESortFinOptimization {
    EOptFinInsert,
    EOptFinQsort,
    EOptFinMerge,
};

class CSimpleArray {
public:
    static const bool LazyFakeArrAlloc = false;

    CSimpleArray(unsigned size, ESortFinOptimization optimization = EOptFinInsert): 
        m_size(size),
        m_k(10),
        m_optimization(optimization)
    {
        LOG("constructor");
        InitArrays();
    }
    CSimpleArray(unsigned size, unsigned k, ESortFinOptimization optimization = EOptFinInsert): 
        m_size(size),
        m_k(k),
        m_optimization(optimization)
    {
        LOG("constructor");
        InitArrays();
    }
    ~CSimpleArray() {
        delete[] m_data;
        if (!LazyFakeArrAlloc) delete[] m_temp;
    }
    void RandomInit() {

    }
    void FillInc() {
        LOG("fill array forward");
        for (unsigned i = 0; i < m_size; ++i) this->Get(i) = i;
    }
    void FillDec() {
        LOG("fill array backward");
        for (unsigned i = 0; i < m_size; ++i) this->Get(i) = m_size - i - 1;
    }
    void FillRand() {
        LOG("fill array randow");
        for (unsigned i = 0; i < m_size; ++i) this->Get(i) = rand();
    }
    TData& operator[](unsigned i) { return m_data[i]; }
    TData& Get(unsigned i) { return m_data[i]; }
    void Dump(std::ostream& out) {
        if (m_size > 0) out << this->Get(0);
        for (unsigned i = 1; i < m_size; ++i) out << ' ' << this->Get(i);
    }
    void MergeSort() {
        LOG("merge sort");
        if (LazyFakeArrAlloc) m_temp = new TData[m_size];
/* create team of threads */
#pragma omp parallel sections       
        {
            MergeSortParallelGo(0, m_size - 1);
        }
        if (LazyFakeArrAlloc) {
            delete[] m_temp;
            m_temp = NULL;
        }
    }
    void QuickSort() {
        LOG("quick sort");
/* create team of threads */
#pragma omp parallel sections
        {
            QuickSortParallelGo(0, m_size - 1);
        }
    }
    void InsertSort() {
        LOG("insert sort");
        InsertSortGo(0, m_size - 1);
    }
    bool ValidateSort() {
        bool res = true;
        for (unsigned i = 1; i < m_size; ++i)
            res &= this->Get(i - 1) <= this->Get(i);
        return res;
    }
    void SetOptimization(ESortFinOptimization optimization) {
        m_optimization = optimization;
    }
protected:
    unsigned m_size;
    TData* m_data;
    TData* m_temp;
    unsigned m_k;
    ESortFinOptimization m_optimization;

    void InitArrays() {
        m_data = new TData[m_size];
        if (LazyFakeArrAlloc) {
            m_temp = NULL;
        } else {
            m_temp = new TData[m_size];
            for (unsigned i = 0; i < m_size; ++i) m_temp[i] = 0;
        }
    }
    void OptimizationGo(unsigned fst, unsigned last) {
        switch (m_optimization) {
        case EOptFinInsert: {
            InsertSortGo(fst, last);
        } break;
        case EOptFinQsort: {
            QuickSortGo(fst, last);
        } break;
        case EOptFinMerge: {
            MergeSortGo(fst, last);
        } break;
        default: {
            assert(0);
        }
        }
    }
    void MergeSortParallelGo(unsigned fst, unsigned last) {
        if (last > fst && last - fst < m_k) {
            OptimizationGo(fst, last);
        } else if (last > fst) {
            unsigned c = (last + fst) / 2;
            {
/* dispatch task to thread */
#pragma omp task
                MergeSortParallelGo(fst, c);
/* dispatch task to thread */
#pragma omp task
                MergeSortParallelGo(c + 1, last);
            }
/* wait task's results */
#pragma omp taskwait
            Merge(fst, c, last);
        }
    }
    void MergeSortGo(unsigned fst, unsigned last) {
        if (last > fst) {
            unsigned c = (last + fst) / 2;
            MergeSortGo(fst, c);
            MergeSortGo(c + 1, last);
            Merge(fst, c, last);
        }
    }    
    void Merge(unsigned fst, unsigned lastFst, unsigned lastSnd) {
        for (unsigned i = fst; i <= lastSnd; ++i) {
            m_temp[i] = m_data[i];
        }
        unsigned snd = lastFst + 1;
        unsigned current = fst;
        while (fst <= lastFst && snd <= lastSnd) {
            if (m_temp[fst] < m_temp[snd]) {
                m_data[current++] = m_temp[fst];
                ++fst;
            } else {
                m_data[current++] = m_temp[snd];
                ++snd;
            }
        }
        while (fst <= lastFst) {
            m_data[current++] = m_temp[fst];
            ++fst;
        }
        while (snd <= lastSnd) {
            m_data[current++] = m_temp[snd];
            ++snd;
        }        
    }
    void QuickSortParallelGo(unsigned fst, unsigned last) {
        if (last > fst && last - fst < m_k) {
            OptimizationGo(fst, last);
        } else if (last > fst) {
            unsigned c = Partition(fst, last);
            {
/* dispatch task to thread */
#pragma omp task
                if (c > 0) QuickSortParallelGo(fst, c - 1);
/* dispatch task to thread */
#pragma omp task
                QuickSortParallelGo(c + 1, last);
            }
        }
    }
    void QuickSortGo(unsigned fst, unsigned last) {
        if (last > fst) {
            unsigned c = Partition(fst, last);
            if (c > 0) QuickSortGo(fst, c - 1);
            QuickSortGo(c + 1, last);
        }
    }
    unsigned Partition(unsigned fst, unsigned last) {
        Swap(last, fst + rand() % (last - fst));
        TData p = m_data[last];
        unsigned less = fst;
        unsigned more = last;
        while (less < more) {
            if (m_data[less] < p) {
                ++less;
            } else {
                Swap(less, --more);
            }
        }
        Swap(last, less);
        return less;
    }
    void Swap(unsigned idx1, unsigned idx2) {
        unsigned tmp = m_data[idx1];
        m_data[idx1] = m_data[idx2];
        m_data[idx2] = tmp;
    }
    void InsertSortGo(unsigned fst, unsigned last) {
        for (unsigned i = fst + 1; i <= last; ++i) {
            unsigned mi = i - 1;
            for (unsigned j = i; j <= last; ++j) {
                if (m_data[j] < m_data[mi]) mi = j;
            }
            Swap(mi, i - 1);
        }
    }
};

std::ostream& operator<<(std::ostream& out, CSimpleArray arr) {
    arr.Dump(out);
    return out;
}
