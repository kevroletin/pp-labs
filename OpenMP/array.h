#include <iostream>
#include <cassert>
#include <sstream>
#include <cstdlib>
#include <omp.h>

typedef unsigned TData;

#define LOG(str)
//#define LOG(str) std::cerr << str << "\n";

//template <class TData> dislike templated due to lazy code generation
class CSimpleArray {
public:
    static const bool LazyFakeArrAlloc = false;
    static const unsigned k = 10;
    
    CSimpleArray(unsigned size): m_size(size) {
        LOG("constructor");
        m_data = new TData[m_size];
        if (LazyFakeArrAlloc) {
            m_temp = NULL;
        } else {
            m_temp = new TData[m_size];
            for (unsigned i = 0; i < m_size; ++i) m_temp[i] = 0;
        }
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
            MergeSortGo(0, m_size - 1);
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
            QuickSortGo(0, m_size - 1);
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
protected:
    unsigned m_size;
    TData* m_data;
    TData* m_temp;

    void MergeSortGo(unsigned fst, unsigned last) {
        if (last - fst < k) {
            InsertSortGo(fst, last);
        } else {
            unsigned c = (last + fst) / 2;
            {
/* dispatch task to thread */
#pragma omp task
                MergeSortGo(fst, c);
/* dispatch task to thread */
#pragma omp task
                MergeSortGo(c + 1, last);
            }
/* wait task's results */
#pragma omp taskwait
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
    void QuickSortGo(unsigned fst, unsigned last) {
        if (last > fst && last - fst < k) {
            InsertSortGo(fst, last);
        } else if (last > fst) {
            unsigned c = Partition(fst, last);
            {
/* dispatch task to thread */
#pragma omp task
                if (c > 0) QuickSortGo(fst, c - 1);
/* dispatch task to thread */
#pragma omp task
                QuickSortGo(c + 1, last);
            }
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
