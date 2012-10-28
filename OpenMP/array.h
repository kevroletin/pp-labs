#include "log.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include <cstdlib>

typedef unsigned TData;

//#define LOG(str) std::cerr << str << "\n";
#define LOG(str) TLogHelper L(m_log, str);

#define DEBUG_LOG(str)
//#define DEBUG_LOG(str) std::cerr << "[DEBUG]" << str << "\n";
//#define DEBUG_LOG(str) TLogHelper L(m_log, std::string("[DEBUG]") + str);

//template <class TData> dislike templated due to lazy code generation
class CSimpleArray {
public:
    static const bool LazyFakeArrAlloc = false;
    
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
        m_temp = new TData[m_size];
        MergeSortGo(0, m_size - 1);
        delete[] m_temp;
    }
    void QuickSort() {
        LOG("quick sort");
        QuickSortGo(0, m_size - 1);
    }
    bool ValidateSort() {
        bool res = true;
        for (unsigned i = 1; i < m_size; ++i)
            res &= this->Get(i - 1) <= this->Get(i);
        return res;
    }
    TLog GetLog() { return m_log; }
protected:
    unsigned m_size;
    TData* m_data;
    TData* m_temp;
    TLog m_log;

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
    void QuickSortGo(unsigned fst, unsigned last) {
        if (last > fst) {
            unsigned c = Partition(fst, last);
            if (c > 0) QuickSortGo(fst, c - 1);
            QuickSortGo(c + 1, last);
        }
    }
    unsigned Partition(unsigned fst, unsigned last) {
        Swap(last, fst + rand() % (last - fst));
        unsigned p = m_data[last];
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
};

std::ostream& operator<<(std::ostream& out, CSimpleArray arr) {
    arr.Dump(out);
    return out;
}
