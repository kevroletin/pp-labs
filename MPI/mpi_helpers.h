#ifndef _MPI_HELPERS
#define _MPI_HELPERS

#include "mpi.h"

#include <list>
#include <ctime>
#include <string>
#include <sstream>
#include <cstring>

//#define STDERR_LOG_DUMP
//#define DEBUG_COMMUNICATION

typedef unsigned uint;
typedef unsigned char uchar;

struct CEnvironment {
    int m_procCnt;
    int m_rank;
    void InitMPI(int cmdLineArgc, char** cmdLineArgv);
    ~CEnvironment() { MPI_Finalize(); }
};

enum EMessageTags {
    MT_COMMANDS,
    MT_DATA_PARAMS,
    MT_DATA_STREAM
};

enum ECommands {
    CMD_NONE,
    CMD_DIE,
    CMD_SEND_TASK,
    CMD_SEND_ANS,
    CMD_SEND_INPUT_DATA,
    CMD_GO,
    CMD_FIN_FOUND,
    CMD_NO_FIN,
    CMD_START_FIND_WAY,
    CMD_FIND_WAY,
    CMD_SEND_WAY,
    CMD_NO_WAY,
    CMD_ENTER,
    CMD_EXIT,
    CMD_MOVE,
    CMD_OK,
    CMD_FAIL,
    CMD_DUMP,
    CMD_IS_CELL_NON_EMPTY,
    CMD_GIVE_CELL_COLOR,
    CMD_IS_CONTAINS_COORD,
    CMD_SET_PIECE,
    CMD_GET_BOARD_INFO,
    CMD_MOVE_BOARD
};

static std::string cmdToStr[] = {
    "CMD_NONE",
    "CMD_DIE",
    "CMD_SEND_TASK",
    "CMD_SEND_ANS",
    "CMD_SEND_INPUT_DATA",
    "CMD_GO",
    "CMD_FIN_FOUND",
    "CMD_NO_FIN",
    "CMD_START_FIND_WAY",
    "CMD_FIND_WAY",
    "CMD_SEND_WAY",
    "CMD_NO_WAY",
    "CMD_ENTER",
    "CMD_EXIT",
    "CMD_MOVE",
    "CMD_OK",
    "CMD_FAIL",
    "CMD_DUMP",
    "CMD_IS_CELL_NON_EMPTY",
    "CMD_GIVE_CELL_COLOR",
    "CMD_IS_CONTAINS_COORD",
    "CMD_SET_PIECE",
    "CMD_GET_BOARD_INFO",
    "CMD_MOVE_BOARD"
};

struct CLogedItem {
    unsigned long long startTimeMs;
    unsigned long long finishTimeMs;
    std::string msg;
};

std::ostream& operator<<(std::ostream& out, CLogedItem logedItem);

struct ILogger {
    virtual void PutToLog(CLogedItem* loggedItem) = 0;
};

class MixTaskLogger: virtual public ILogger {
public:
    MixTaskLogger(int ownerRank, std::string name = "");
    void PublishLog();
    void PublishLog(std::ostream& out);
    void SendLog();
    virtual void PutToLog(CLogedItem* loggedItem);
protected:
    int m_ownerRank;
    std::list<CLogedItem*> m_buffer;
    std::string m_name;
    void PublishItem(std::ostream& out, CLogedItem* li);    
};

class MixSlaveLogger: virtual public ILogger {
public:
    MixSlaveLogger(ILogger& masterLog): m_masterLog(masterLog) {}
    virtual void PutToLog(CLogedItem* loggedItem) { m_masterLog.PutToLog(loggedItem); }
protected:
    ILogger& m_masterLog;
};

class CLogHelper {
public:
    CLogHelper(ILogger& log, std::string msg);
    ~CLogHelper();

protected:
    CLogedItem* m_logedItem;
};

struct IHaveRank {
    virtual int GetRank() = 0;
    bool IsSupervisor() { return GetRank() == 0; }
};

class CRankOwner: virtual public IHaveRank {
public:
    CRankOwner(int rank): m_rank(rank) {}
    virtual int GetRank() { return m_rank; }
protected:
    int m_rank;
};

class CRankSlave: virtual public IHaveRank {
public:
    CRankSlave(IHaveRank& owner): m_owner(owner) {}
    virtual int GetRank() { return m_owner.GetRank(); }
protected:
    IHaveRank& m_owner;
};

class CMpiConnections;
class CSquareField;
class CMatrix;
class CSideCoord;
class CPointCoord;
class CCoord2D;
class CCoord3D;
class CBoard;

struct MixMpiHelper: virtual public IHaveRank, virtual public ILogger {
    void SendCmd(unsigned cmd, int destTask = 0);
    ECommands RecieveCmd(int sourceTask = 0, MPI_Status* pStatus = NULL);
    void RecieveAndCheckCmd(ECommands cmd, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(unsigned& result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(int&      result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(double&   result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(CMpiConnections& result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(CSquareField&    result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(CMatrix&         result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(CSideCoord&      result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(CPointCoord&     result, int sourceTask, MPI_Status* pStatus = NULL);
    void GetData(std::string&     result, int sourceTask, MPI_Status* pStatus = NULL);    
    void GetData(CCoord2D&        result, int sourceTask, MPI_Status* pStatus = NULL);    
    void GetData(CCoord3D&        result, int sourceTask, MPI_Status* pStatus = NULL);    
    void SendData(unsigned data, int destTask);
    void SendData(int      data, int destTask);
    void SendData(double   data, int destTask);
    void SendData(CMpiConnections data, int destTask);
    void SendData(CSquareField&   data, int destTask);
    void SendData(CMatrix         data, int destTask);
    void SendData(CSideCoord      data, int destTask);
    void SendData(CPointCoord     data, int destTask);
    void SendData(std::string     data, int destTask);
    void SendData(CCoord2D        data, int destTask);
    void SendData(CCoord3D        data, int destTask);
    void GetDataBcast(unsigned& result);
    void SendDataBcast(unsigned data);
    void ThrowBadCmd(ECommands expectedCmd, ECommands gotCmd);
};

#endif // WORKERS_MPI_H
