#ifndef _MPI_HELPERS
#define _MPI_HELPERS

#include "mpi.h"

#include <list>
#include <ctime>
#include <string>
#include <sstream>
#include <cstring>

//#define STDERR_LOG_DUMP

typedef unsigned uint;
typedef unsigned char uchar;

struct TEnvironment {
    int m_procCnt;
    int m_rank;
    void InitMPI(int cmdLineArgc, char** cmdLineArgv);
    ~TEnvironment() { MPI_Finalize(); }
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
};

static std::string cmdToStr[] = {
    "CMD_NONE",
    "CMD_DIE",
    "CMD_SEND_TASK",
    "CMD_SEND_ANS",
    "CMD_SEND_INPUT_DATA",
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
//    bool IsSupervisor() { return GetRank() == 0; }
protected:
    int m_rank;
};

class CMpiConnections;
class CSquareField;
class CMatrix;

struct MixMpiHelper: virtual public IHaveRank, virtual public ILogger {
    void SendCmd(unsigned cmd, int destTask = 0);
    ECommands RecieveCmd(int sourceTask = 0, MPI_Status* pStatus = NULL);
    void RecieveAndCheckCmd(ECommands cmd, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(unsigned& result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(int&      result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(double&   result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(CMpiConnections& result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(CSquareField&    result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(CMatrix&         result, int sourceTask = 0, MPI_Status* pStatus = NULL);
    void SendData(unsigned data, int destTask = 0);
    void SendData(int      data, int destTask = 0);
    void SendData(double   data, int destTask = 0);
    void SendData(CMpiConnections data, int destTask = 0);
    void SendData(CSquareField&   data, int destTask = 0);
    void SendData(CMatrix         data, int destTask = 0);
    void GetDataBcast(unsigned& result);
    void SendDataBcast(unsigned data);
    void ThrowBadCmd(ECommands expectedCmd, ECommands gotCmd);
};

#endif // WORKERS_MPI_H
