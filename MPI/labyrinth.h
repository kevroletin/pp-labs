#ifndef _LABYRINTH_H
#define _LABYRINTH_H

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <queue>
#include <cassert>
#include <fstream>

typedef unsigned uint;

#define Log(str) CLogHelper L(*this, str);
#define LogEx(data)                              \
    {                                            \
        std::stringstream ss;                    \
        ss << data;                              \
        Log(ss.str());                           \
    }
#if 0
#    define AlgoLog(str) Log(str)
#    define AlgoLogEx(data) LogEx(data)
#else
#    define AlgoLog(str)
#    define AlgoLogEx(data)
#endif

enum ESide { ETop, ERight, EBottom, ELeft };
extern int moveDx[];
extern int moveDy[];
extern std::string sideStr[];

ESide Invert( ESide side );

struct CPointCoord {
    CPointCoord() {}
    CPointCoord(int x, int y): m_x(x), m_y(y) {}
    int m_x, m_y;
    CPointCoord Move(ESide side) { 
        return CPointCoord(m_x + moveDx[side], m_y + moveDy[side]);
    }
};

std::ostream& operator<<(std::ostream& out, CPointCoord point);

struct CSideCoord { 
    CSideCoord() {}
    CSideCoord(uint offset, ESide side): m_offset(offset), m_side(side) {}
    uint m_offset;
    ESide m_side; 
    CSideCoord Flip() {
        return CSideCoord( m_offset, Invert(m_side) );
    }
    bool operator==(const CSideCoord& c) const {
        return m_offset == c.m_offset && m_side == c.m_side;
    }
};

std::ostream& operator<<(std::ostream& out, CSideCoord coord);

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
    bool operator==(const CSize& s) { return m_x == s.m_x && m_y == s.m_y; }
};

inline std::ostream& operator<<(std::ostream& out, CSize size)
{
    out << "x: " << size.m_x << " y: " << size.m_y;
    return out;
}
std::ostream& operator<<(std::ostream& out, CSideCoord coord);

const uint Empty = 0;
const uint Full  = 1;
const uint Start = 2;
const uint Fin   = 3;

struct CMatrix {
    uint& Get(uint x, uint y) {
        if (x >= m_dim.m_x || y >= m_dim.m_y) {
            std::stringstream ss;
            ss << "Error: dim " << m_dim << "; requested: " << CPointCoord(x, y);
            throw ss.str();
        }
        return m_data[y*m_dim.m_x + x];
    }
    uint GetConst(uint x, uint y) const {
        if (x >= m_dim.m_x || y >= m_dim.m_y) {
            std::stringstream ss;
            ss << "Error: dim " << m_dim << "; requested: " << CPointCoord(x, y);
            throw ss.str();
        }
        return m_data[y*m_dim.m_x + x];
    }
    void Resize(CSize dim, uint value = 0) {
        m_dim = dim;
        m_data.resize(m_dim.m_x * m_dim.m_y, value);
    }
    void Resize(uint value = 0) {
        m_data.resize(m_dim.m_x * m_dim.m_y, value);
    }
    bool Inside(int x, int y) { return m_dim.Inside(x, y); }
    void Dump(std::ostream& out) const {
        for (uint i = 0; i < m_dim.m_y; ++i) {
            for (uint j = 0; j < m_dim.m_x; ++j) {
                uint v = GetConst(j, i);
                if (v == (uint)-1) {
                    out << "# ";
                } else {
                    out << v << " ";
                }
            }
            out << "\n";
        }
    }
    bool operator==(const CMatrix& m) {
        if (!(m_dim == m.m_dim)) return false;
        for (uint y = 0; y < m_dim.m_y; ++y) {
            for (uint x = 0; x < m_dim.m_x; ++x) {
                if (GetConst(x, y) != m.GetConst(x, y)) return false;
            }
        }
        return true;
    }
    CSize m_dim;
    std::vector<uint> m_data;
};

inline std::ostream& operator<<(std::ostream& out, const CMatrix& matr) {
    matr.Dump(out);
    return out;
}

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
    CSquareField(ICommunicator* comm, ILogger& masterLog): MixSlaveLogger(masterLog), m_comm(comm) {}
    void SetCommunicator(ICommunicator* comm) { m_comm = comm; }
    void Resize(uint len) { Resize(CSize(len, len)); }
    void Resize(CSize size) {
        assert(size.m_x == size.m_y);
        m_size = size;
        m_type.Resize(size, 0);
        m_data.Resize(size, -1);
    }
    std::string GetSymbol(uint x, uint y) const { 
        uint t = GetTypeConst(x, y);
        if (Full  == t) return "#";
        if (Start == t) return "S";
        if (Fin   == t) return "F";
        if ((uint)-1 == GetDataConst(x, y)) return "-";
        std::stringstream ss;
        ss << GetDataConst(x, y);
        return ss.str();
    }
    CMatrix& GetType() { return m_type; }
    uint& GetType(uint x, uint y) { return m_type.Get(x, y); }
    uint  GetTypeConst(uint x, uint y) const { return m_type.GetConst(x, y); }
    uint& GetType(CPointCoord p) { return m_type.Get(p.m_x, p.m_y); }
    CMatrix& GetData() { return m_data; }
    uint& GetData(uint x, uint y) { return m_data.Get(x, y); }
    uint  GetDataConst(uint x, uint y) const { return m_data.GetConst(x, y); }
    uint& GetData(CPointCoord p) { return m_data.Get(p.m_x, p.m_y); }
    
    void Dump(std::ostream& out, bool pretty = true) const {
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
        return GoDFS(value, coord.m_x, coord.m_y);
/* Hard to implement bfs on multithread grid => use dfs everywhere */
/*      LogEx("Go " << coord);
        if (!PushBFSQueue(value, coord.m_x, coord.m_y)) return false;
        GoBFS(-1);
        return true; */
    }
    virtual bool GetWay(uint value, CPointCoord coord, std::string& way) {
        if (value == (uint)-1) value = GetData(coord) + 1;
        { AlgoLogEx("GetWay " << value << " | " << coord); }
        
        if (!m_size.Inside(coord)) {
            AlgoLog("Go outside");
            return m_comm->GetWay(value, m_size.ToSideCoord(coord), way);
        }
        AlgoLogEx("Cell dist value " << GetData(coord));

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
        return false;
    }
    CSize GetSize() { return m_size; }
    bool operator ==(const CSquareField& f) {
        return m_size == f.m_size && 
               m_type == f.m_type &&
               m_data == f.m_data;
    }
protected:
    ICommunicator* m_comm;
    CSize m_size;
    CMatrix m_type;
    CMatrix m_data;
    std::queue<CPointCoord> m_q;
    void GoBFS(uint stepsLimit) {
        while (!m_q.empty() && stepsLimit-- != 0) {
            CPointCoord p = m_q.front(); m_q.pop();
            LogEx("BFS process point " << p);
            uint x = p.m_x, y = p.m_y;
            uint dist = m_data.Get(x, y);
            PushBFSQueue(dist + 1, x + 1, y    );
            PushBFSQueue(dist + 1, x - 1, y    );
            PushBFSQueue(dist + 1, x    , y + 1);
            PushBFSQueue(dist + 1, x    , y - 1);
        }
    }
    bool PushBFSQueue(uint dist, int x, int y) {
        LogEx("Push BFS queue " << dist << " " << CPointCoord(x, y));
        if (m_size.Inside(x, y)) {
            if (Full == GetType(x, y)) return false;
            if (Fin == GetType(x, y)) {
                Log("Fin found");
                if (dist < GetData(x, y)) GetData(x, y) = dist;
                return false;
            }
            if (dist < GetData(x, y)) {
                Log("Update value");
                GetData(x, y) = dist;
                m_q.push(CPointCoord(x, y));
            }
            return true;
        } else {
            LogEx("Go outside " << CPointCoord(x, y));
            assert(NULL != m_comm);
            return m_comm->Go(dist, m_size.ToSideCoord(x, y));
        }
    }
    bool GoDFS(uint currDist, int x, int y) {
        AlgoLogEx("GoDFS" << " x: " << x << " y: " << y);
        if (m_size.Inside(x, y)) {
            if (Fin == GetType(x, y)) {
                AlgoLogEx("GoDFS: Fin found x: " << x << " y: " << y);
                if (currDist < GetData(x, y)) {
                    AlgoLogEx("GoDFS: updated fin value " << GetData(x, y) << " - " << currDist);
                    GetData(x, y) = currDist;
                }
                return true;
            }
            if (m_type.Get(x, y) != Full && currDist < m_data.Get(x, y)) {
                AlgoLog("GoDFS: update");
                m_data.Get(x, y) = currDist;
                bool res = false;
                res |= GoDFS(currDist + 1, x - 1, y    );
                res |= GoDFS(currDist + 1, x + 1, y    );
                res |= GoDFS(currDist + 1, x,     y + 1);
                res |= GoDFS(currDist + 1, x,     y - 1);
                return res;
            }
        } else {
            AlgoLogEx("Go outside " << CPointCoord(x, y));
            assert(NULL != m_comm);
            return m_comm->Go(currDist, m_size.ToSideCoord(x, y));
        }
        return false;
    }
};

inline std::ostream& operator<<(std::ostream& out, const CSquareField field) {
    field.Dump(out);
    return out;
}

class CFieldReder: public MixSlaveLogger {
public:
    CPointCoord m_startSquareCoord;
    CPointCoord m_startGridItem;
    CPointCoord m_finSquareCoord;
    CPointCoord m_finGridItem;

    CFieldReder(ILogger& masterLog): MixSlaveLogger(masterLog) {} 
    void SetDimentions(uint n, uint k) {
        m_gridSize = k;
        m_squareSize = n / k;
        assert(0 == n % k);
        m_data.resize(m_gridSize*m_gridSize, NULL);
        LogEx("CFieldReder: gridSize " << m_gridSize << " squareSize " << m_squareSize);
    }
    void InitGrid() {
        {
            LogEx("CFieldReder: Create fields");
            for (uint y = 0; y < m_gridSize; ++y) {
                for (uint x = 0; x < m_gridSize; ++x) {
                    std::stringstream ss;
                    ss << "field [" << y << "][" << x << "]";
                    Get(x, y) = new CSquareField(NULL, *this);
                    Get(x, y)->Resize(m_squareSize);
                }
            }
        }
        {
            Log("CFieldReder: Create communicators");
            for (uint y = 0; y < m_gridSize; ++y) {
                for (uint x = 0; x < m_gridSize; ++x) {
                    uint n = m_gridSize;
                    IField* ff[4] = {
                        (y > 0     ? Get(x,   y-1) : NULL),
                        (x < n - 1 ? Get(x+1, y  ) : NULL),
                        (y < n - 1 ? Get(x,   y+1) : NULL),
                        (x > 0     ? Get(x-1, y  ) : NULL) };
                    
                    Get(x, y)->SetCommunicator( new CSimpleCommunicator(ff[0], ff[1], ff[2], ff[3], *this));
                }
            }
        }
    }
    bool Read(std::string fname) {
        std::ifstream in;
        in.open(fname.c_str());
        bool res = Read(in);
        in.close();
        return res;
    }
    bool Read(std::istream& in) {
        Log("CFieldReder: Read");
        uint n, k;
        in >> n >> k;
        SetDimentions(n, k);
        InitGrid();

        bool wasStart = false;
        bool wasFin = false;
        for (uint grid_y = 0; grid_y < m_gridSize; ++grid_y) {
            for (uint y = 0; y < m_squareSize; ++y) {
                for (uint grid_x = 0; grid_x < m_gridSize; ++grid_x) {
                    CSquareField* f = Get(grid_x, grid_y);
                    for (uint x = 0; x < m_squareSize; ++x) {
                        char c;
                        in >> c;
                        switch (c) {
                        case '#': {
                            f->GetType(x, y) = Full;
                        } break;
                        case '.': {
                            f->GetType(x, y) = Empty;
                        } break;
                        case 'S': {
                            assert( !wasStart);
                            wasStart = true;
                            f->GetType(x, y) = Start;
                            m_startSquareCoord = CPointCoord(x, y);
                            m_startGridItem = CPointCoord(grid_x, grid_y);
                        } break;
                        case 'F': {
                            assert( !wasFin);
                            wasFin = true;
                            f->GetType(x, y) = Fin;
                            m_finSquareCoord = CPointCoord(x, y);
                            m_finGridItem = CPointCoord(grid_x, grid_y);
                        } break;
                        default: { assert(0); }
                        }
                    }
                }
            }
        }
        assert( wasStart );
        assert( wasFin );
        return true;
    }
    void Dump(std::ostream& out, bool pretty = true) {
        for (uint grid_y = 0; grid_y < m_gridSize; ++grid_y) {
            for (uint y = 0; y < m_squareSize; ++y) {
                for (uint grid_x = 0; grid_x < m_gridSize; ++grid_x) {
                    CSquareField* f = Get(grid_x, grid_y);
                    for (uint x = 0; x < m_squareSize; ++x) {
                        if (pretty) out << f->GetSymbol(x, y) << " ";
                        else {
                            if ((uint)-1 == f->GetData(x, y)) out << "- ";
                            else out << f->GetData(x, y) << " ";
                        }
                    }
                    out << " | ";
                }
                out << "\n";
            }
            for (uint k = 0; k < m_gridSize * (m_squareSize + 3) - 1; ++k)
                out << "-";
            out << "\n";
        }    
    }
    CSquareField*& Get(uint x, uint y) { return m_data[y*m_gridSize + x]; }    
    CSquareField*& Get(CPointCoord p) { return Get(p.m_x, p.m_y); }
    uint GetGridSize() { return m_gridSize; }
    uint GetTotalSize() { return m_squareSize*m_gridSize; }
    bool FindWay(std::string& way) {
        CPointCoord s  = m_startSquareCoord;
        CPointCoord sg = m_startGridItem;
        CPointCoord f  = m_finSquareCoord;
        CPointCoord fg = m_finGridItem;
        Get(sg)->Go(0, s);
        if (Get(fg)->GetData(f) != (uint)-1) {
            Log("=== Find Way ===");
            if (Get(fg)->GetWay(-1, f, way)) {
                return true;
            }
        }
        return false;
    }
protected:
    std::vector<CSquareField*> m_data;
    uint m_gridSize;
    uint m_squareSize;
    
};

inline std::ostream& operator<<(std::ostream& out, CFieldReder& reader)
{
    reader.Dump(out);
    return out;
}

struct CMpiConnections {
    CMpiConnections() {}
    CMpiConnections(uint gridSize, CPointCoord top, CPointCoord right, CPointCoord bottom, CPointCoord left) {
        m_topRank = GridCoordToRank(gridSize, top);
        m_rightRank = GridCoordToRank(gridSize, right);
        m_bottomRank = GridCoordToRank(gridSize, bottom);
        m_leftRank = GridCoordToRank(gridSize, left);
    }
    CMpiConnections(int topRank, int rightRank, int bottomRank, int leftRank) {
        m_topRank = topRank;
        m_rightRank = rightRank;
        m_bottomRank = bottomRank;
        m_leftRank = leftRank;
    }    
    static int GridCoordToRank(uint gridSize, CPointCoord p) {
        return CSize(gridSize, gridSize).Inside(p) ?
            1 + gridSize * p.m_y + p.m_x : 
            0;
    }
    int m_topRank;
    int m_rightRank;
    int m_bottomRank;
    int m_leftRank;
};

#endif
