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
    int ProcCnt;
    int Rank;
    void InitMPI(int CmdLineArgc, char** CmdLineArgv);
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

static std::string CmdToStr[] = {
    "CMD_NONE",
    "CMD_DIE",
    "CMD_SEND_TASK",
    "CMD_SEND_ANS",
    "CMD_SEND_INPUT_DATA",
};

struct ITask {
    virtual int GetRank() = 0;
    bool IsSupervisor() { return GetRank() == 0; }
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

class MixTaskLogger: public ILogger {
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

class MixSlaveLogger: public ILogger {
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
//    ILogger& m_log;
    CLogedItem* m_logedItem;
};

struct IMpiHelper: public ITask {
    void SendCmd(unsigned Cmd, int DestTask = 0);
    ECommands RecieveCmd(int SourceTask = 0, MPI_Status* pStatus = NULL);
    void RecieveAndCheckCmd(ECommands Cmd, int SourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(unsigned& Result, int SourceTask = 0, MPI_Status* pStatus = NULL);
    void GetData(double&   Result, int SourceTask = 0, MPI_Status* pStatus = NULL);
    void SendData(unsigned Data, int DestTask = 0);
    void SendData(double   Data, int DestTask = 0);
    void GetDataBcast(unsigned& Result);
    void SendDataBcast(unsigned Data);
    void ThrowBadCmd(ECommands ExpectedCmd, ECommands GotCmd);
};

#endif // WORKERS_MPI_H
