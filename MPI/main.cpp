#include "mpi_helpers.h"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <cassert>
#include <iomanip>
#include <queue>

//#define LOG(str) PutToLog(str);
//#define LOG(str) std::cerr << str << "\n";

//#define SUPERVISOR_LOG(str)
//#define SUPERVISOR_LOG(str) std::cerr << "[0] " << str << "\n";
#define Log(str) CLogHelper L(*this, str);
#define LogEx(data)                              \
    {                                            \
        std::stringstream ss;                    \
        ss << data;                              \
        Log(ss.str());                           \
    }


enum ESide { ETop, ERight, EBottom, ELeft };
int moveDx[] = { 0, 1, 0, -1 };
int moveDy[] = { -1, 0, 1, 0 };
std::string sideStr[] = { "ETop", "ERight", "EBottom", "ELeft" };

ESide Invert( ESide side ) { 
    return static_cast<ESide>( (side + 2) % 4 );
}

struct CPointCoord {
    CPointCoord(int x, int y): m_x(x), m_y(y) {}
    int m_x, m_y;
    CPointCoord Move(ESide side) { 
        return CPointCoord(m_x + moveDx[side], m_y + moveDy[side]);
    }
};

std::ostream& operator<<(std::ostream& out, CPointCoord point) {
    out << "x: " << point.m_x << ", y:" << point.m_y;
    return out;
}

struct CSideCoord { 
    CSideCoord(uint offset, ESide side): m_offset(offset), m_side(side) {}
    uint m_offset;
    ESide m_side; 
    CSideCoord Flip() {
        return CSideCoord( m_offset, Invert(m_side) );
    }
};

std::ostream& operator<<(std::ostream& out, CSideCoord coord) {
    out << "side: " << sideStr[coord.m_side] << ", offset:" << coord.m_offset;
    return out;
}

struct CSize { 
    CSize() {}
    CSize(uint x, uint y): m_x(x), m_y(y) {}
    uint m_x, m_y;
    bool Inside(int x, int y) {
        return x >= 0 && (uint)x < m_x && y >= 0 && (uint)y < m_y;
    }
    bool Inside(CPointCoord p) {
        return Inside(p.m_x, p.m_y);
    }
    CSideCoord ToSideCoord(CPointCoord p) {
        return ToSideCoord(p.m_x, p.m_y);
    }
    CSideCoord ToSideCoord(int x, int y) {
        if (x > 0 && (uint)x >= m_x) {
            assert( y >= 0 && (uint)y < m_y );
            return CSideCoord(y, ERight);
        }
        if (x < 0) {
            assert( y >= 0 && (uint)y < m_y );
            return CSideCoord(y, ELeft);
        }
        if (y > 0 && (uint)y >= m_y) {
            assert( x >= 0 && (uint)x < m_x );
            return CSideCoord(x, EBottom);
        }
        if (y < 0) {
            assert( x >= 0 && (uint)x < m_x );
            return CSideCoord(x, ETop);
        }
        assert(0);
    }
    CPointCoord FromSideCoord(CSideCoord coord) {
        switch (coord.m_side) {
        case ETop: {
            return CPointCoord(coord.m_offset, 0);
        } break;
        case ERight: {
            return CPointCoord(m_x - 1, coord.m_offset);
        } break;
        case EBottom: {
            return CPointCoord(coord.m_offset, m_y - 1);
        } break;
        case ELeft: {
            return CPointCoord(0, coord.m_offset);
        } break;
        default: assert(0);
        }
    }
};

std::ostream& operator<<(std::ostream& out, CSize s) {
    out << "x: " << s.m_x << " y:" << s.m_y;
    return out;
}


const uint Empty = 0;
const uint Full  = 1;
const uint Start = 2;
const uint Fin   = 3;

class CMatrix {
public:
    uint& Get(uint x, uint y) {
        return m_data[y*m_dim.m_x + x];
    }
    void Resize(CSize dim, uint value = 0) {
        m_dim = dim;
        m_data.resize(m_dim.m_x * m_dim.m_y, value);
    }
    bool Inside(int x, int y) { return m_dim.Inside(x, y); }
    void Dump(std::ostream& out) {
        for (uint i = 0; i < m_dim.m_y; ++i) {
            for (uint j = 0; j < m_dim.m_x; ++j) {
                uint v = Get(j, i);
                if (v == (uint)-1) {
                    out << "# ";
                } else {
                    out << v << " ";
                }
            }
            out << "\n";
        }
    }
protected:
    CSize m_dim;
    std::vector<uint> m_data;
};

struct ICommunicator {
    virtual bool Go(uint value, CSideCoord coord) = 0;
    virtual bool GetWay(uint value, CSideCoord coord, std::string& way) = 0;
};

struct IField {
    virtual bool Go(uint value, CPointCoord coord) = 0;
    virtual bool GetWay(uint value, CPointCoord coord, std::string& way) = 0;
    virtual CSize GetSize() = 0;
    
};

struct CSimpleCommunicator: public ICommunicator, public MixSlaveLogger {
    CSimpleCommunicator(IField* top, IField* right, IField* bottom, IField* left, ILogger& masterLog):
        MixSlaveLogger(masterLog)
    {
        m_fields[ETop] = top;
        m_fields[ERight] = right;
        m_fields[EBottom] = bottom;
        m_fields[ELeft] = left;
    }
    IField* m_fields[4];
    virtual bool Go(uint value, CSideCoord coord) {
        LogEx("Comm Go " << coord);
        IField* field = m_fields[coord.m_side];
        if (NULL == field) return false;
        
        CPointCoord point = field->GetSize().FromSideCoord(coord.Flip());
        LogEx(coord.Flip() << " | " << point);
        assert( field->GetSize().Inside(point) );
        return field->Go( value, point );
    }
    virtual bool GetWay(uint value, CSideCoord coord, std::string& way) {
        LogEx("Comm GetWay " << coord);
        IField* field = m_fields[coord.m_side];
        if (NULL == field) return false;
        
        CPointCoord point = field->GetSize().FromSideCoord(coord.Flip());
        LogEx(coord.Flip() << " | " << point);
        assert( field->GetSize().Inside(point) );
        return field->GetWay(value, point, way);        
    }
};

struct CDebugCommunicator: public ICommunicator {
    virtual bool Go(uint value, CSideCoord coord) {
        std::cerr << "Go: value " << value << " offset " << coord.m_offset <<
                     " side " << sideStr[coord.m_side] << "\n";
        return false;
    }
    virtual bool GetWay(uint value, CSideCoord coord, std::string& way) {
        std::cerr << "GetWay: value " << value << " offset " << coord.m_offset <<
            " side " << sideStr[coord.m_side] << "\n";
        return false;
    }
};
    
class CSquareField: public IField, public MixSlaveLogger {
public:
    CSquareField(ICommunicator* comm, ILogger& masterLog): MixSlaveLogger(masterLog), m_comm(comm) { Log("Good"); }
    void SetCommunicator(ICommunicator* comm) { m_comm = comm; }
    void Resize(uint x, uint y) { Resize(CSize(x, y)); }
    void Resize(CSize size) {
        m_size = size;
        m_type.Resize(size, 0);
        m_data.Resize(size, -1);
    }
    std::string GetSymbol(uint x, uint y) { 
        uint t = GetType(x, y);
        if (Full  == t) return "#";
        if (Start == t) return "S";
        if (Fin   == t) return "F";
        if ((uint)-1 == GetData(x, y)) return "-";
        std::stringstream ss;
        ss << GetData(x, y);
        return ss.str();
    }
    uint& GetType(uint x, uint y) { return m_type.Get(x, y); }
    uint& GetType(CPointCoord p) { return m_type.Get(p.m_x, p.m_y); }
    uint& GetData(uint x, uint y) { return m_data.Get(x, y); }
    uint& GetData(CPointCoord p) { return m_data.Get(p.m_x, p.m_y); }
    
    void Dump(std::ostream& out, bool pretty = true) {
        if (pretty) {
            for (uint y = 0; y < m_size.m_y; ++y) {
                for (uint x = 0; x < m_size.m_x; ++x) {
                    out << GetSymbol(x, y) << ' ';
                }
                out << "\n";
            }
        } else {
            out << "=== data ===\n";
            m_data.Dump(out);
            out << "=== type ===\n";
            m_type.Dump(out);
        }
    }
    virtual bool Go(uint value, CPointCoord coord) { 
        //GoDFS(value, coord.m_x, coord.m_y);
        LogEx("Go " << coord);
        PushBFSQueue(value, coord.m_x, coord.m_y);
        return GoBFS(-1);
    }
    virtual bool GetWay(uint value, CPointCoord coord, std::string& way) {
//        LogEx("GetWay " << value << " | " << coord);
        if (value == (uint)-1) value = GetData(coord) + 1;

        if (!m_size.Inside(coord)) {
            Log("Go outside");
            if (m_comm->GetWay(value, m_size.ToSideCoord(coord), way)) return true;
        } else {
            if (Start == GetType(coord)) return true;
            if (value <= GetData(coord)) return false;
            assert(value == GetData(coord) + 1);

            way.push_back('N');
            if (GetWay(value - 1, coord.Move(ETop), way)) return true;
            way.resize(way.size() - 1);
            way.push_back('W');
            if (GetWay(value - 1, coord.Move(ERight), way)) return true;
            way.resize(way.size() - 1);
            way.push_back('S');
            if (GetWay(value - 1, coord.Move(EBottom), way)) return true;
            way.resize(way.size() - 1);
            way.push_back('E');
            if (GetWay(value - 1, coord.Move(ELeft), way)) return true;
            way.resize(way.size() - 1);
        }
        return false;
    }
    virtual CSize GetSize() { return m_size; }
protected:
    ICommunicator* m_comm;
    CSize m_size;
    CMatrix m_type;
    CMatrix m_data;
    std::queue<CPointCoord> m_q;
    bool GoBFS(uint stepsLimit) {
        while (!m_q.empty() && stepsLimit-- != 0) {
            CPointCoord p = m_q.front(); m_q.pop();
            LogEx("BFS process point " << p);
            uint x = p.m_x, y = p.m_y;
            uint dist = m_data.Get(x, y);
            bool res = 
                    PushBFSQueue(dist + 1, x + 1, y    ) ||
                    PushBFSQueue(dist + 1, x - 1, y    ) ||
                    PushBFSQueue(dist + 1, x    , y + 1) ||
                    PushBFSQueue(dist + 1, x    , y - 1);
            if (res) return true;
        }
        return false;
    }
    bool PushBFSQueue(uint dist, int x, int y) {
        LogEx("Push BFS queue " << dist << " " << CPointCoord(x, y));
        if (m_size.Inside(x, y)) {
            if (Fin == GetType(x, y)) {
                GetData(x, y) = dist;
                return true;
            }
            
            if (dist < GetData(x, y)) {
                GetData(x, y) = dist;
                m_q.push(CPointCoord(x, y));
            }
            return false;
        } else {
            LogEx("Go outside " << CPointCoord(x, y));
            assert(NULL != m_comm);
            return m_comm->Go(dist, m_size.ToSideCoord(x, y));
        }
    }
    bool GoDFS(uint currDist, int x, int y) {
        if (m_size.Inside(x, y)) {
            if (Fin == GetType(x, y)) {
                GetData(x, y) = currDist;
                return true;
            }
            if (m_type.Get(x, y) != Full && currDist < m_data.Get(x, y)) {
                m_data.Get(x, y) = currDist;
                return
                    GoDFS(currDist + 1, x - 1, y    ) ||
                    GoDFS(currDist + 1, x + 1, y    ) ||
                    GoDFS(currDist + 1, x,     y + 1) ||
                    GoDFS(currDist + 1, x,     y - 1);
            }
        } else {
            LogEx("Go outside " << CPointCoord(x, y));
            assert(NULL != m_comm);
            return m_comm->Go(currDist, m_size.ToSideCoord(x, y));
        }
        return false;
    }
};

class MixStderrLogger: public ILogger {
    virtual void PutToLog(CLogedItem* loggedItem) {
        std::cerr << loggedItem << "\n";
    }
};

template <class T>
void DumpGroupOfTables(T f, uint n, uint size) {
    for (uint y = 0; y < n; ++y) {
        for (uint yi = 0; yi < size; ++ yi) {
            for (uint x = 0; x < n; ++x) {
                for (uint xi = 0; xi < size; ++xi) {
                    uint u = f[y][x]->GetData(xi, yi); // not good indexing (
                    if (u == (uint)-1) {
                        std::cerr << "# ";
                    } else {
                        std::cerr << u << " ";
                    }
                }
                std::cerr << ". ";
            }
            std::cerr << "\n";
        }
        for (uint k = 0; k < n * (size + 1) - 1; ++k)
            std::cerr << "--";
        std::cerr << "\n";
    }    
}

int main(int argc, char* argv[])
{
    if (1) {
        CSize s(4, 4);
        for (int y = -2; y <= 5; ++y)
            for (int x = -2; x <= 5; ++x)
            {
                bool inside = s.Inside(CPointCoord(x, y));
                if (y >= 0 && y <= 3 && x >= 0 && x <= 3) {
                    assert(inside);
                } else {
                    assert(!inside);
                }
            }
    }
    if (0) {
        ILogger* l = new MixTaskLogger(0);
        CSquareField f(NULL, *l);
        f.SetCommunicator(new CSimpleCommunicator(0, 0, 0, 0, *l));
        f.Resize(4, 4);
        for (uint i = 0; i < 4; ++i) 
            for (uint j = 0; j < 4; ++j) {
                f.GetType(i, j) = Empty;
                f.GetData(i, j) = (uint)-1;
            }
        f.Dump(std::cerr);

        f.GetType(1, 1) = Start;
        f.Go(0, CPointCoord(1, 1));
        f.Dump(std::cerr);
        f.Dump(std::cerr, false);
        std::string way;
        f.GetWay(-1, CPointCoord(3, 0), way);
        std::cerr << way;
        return 0;
    }
    unsigned size = 3;

    const int n = 5;
    CSquareField* f[n][n];
    std::cerr << ("Create fields");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::stringstream ss;
            ss << "field [" << i << "][" << j << "]";
            ILogger* l = new MixTaskLogger(0, ss.str());
            f[i][j] = new CSquareField(NULL, *l);
            f[i][j]->Resize(size, size);
        }
    }
    
    std::cerr <<("Create communicators");    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            IField* ff[4] = {
                (i > 0 ? f[i-1][j] : NULL),
                (j < n - 1? f[i][j+1] : NULL),
                (i < n - 1 ? f[i+1][j] : NULL),
                (j > 0 ? f[i][j-1] : NULL) };
            std::stringstream ss;
            ss << "comm [" << i << "][" << j << "]";
            ILogger* l = new MixTaskLogger(0, ss.str());
            f[i][j]->SetCommunicator( new CSimpleCommunicator(ff[0], ff[1], ff[2], ff[3], *l));
        }
    }

    DumpGroupOfTables(f, n, size);
    CPointCoord start(1, 1);
    f[1][0]->GetType(start) = Start;
    f[1][0]->Go(0, start);
    std::cerr << "----\n";
    DumpGroupOfTables(f, n, size);
    std::string way;
    f[0][0]->GetWay(-1, CPointCoord(0, 0), way);
    std::cerr << way << "\n";
    
    
    return 0;
}
