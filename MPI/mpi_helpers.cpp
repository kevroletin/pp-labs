#include "mpi_helpers.h"
#include "mpi.h"

#include <cassert> // TODO: throw errors enstead
#include <fstream>
#include <sstream>

void TEnvironment::InitMPI(int CmdLineArgc, char** CmdLineArgv)
{
    MPI_Init(&CmdLineArgc, &CmdLineArgv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcCnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
}

/*
TEdgePoint TSupervisor::ReceiveResults()
{
    SUPERVISOR_LOG("Recieve results");
    TEdgePoint BestResult = { INF, INF, -1 };
    for (unsigned i = 0; i < Graph.GetEdgesCnt(); ++i) {
        MPI_Status Status;
        RecieveAndCheckCmd(CMD_SEND_ANS_MAIN, MPI_ANY_SOURCE, &Status);
        TEdgePoint NewResult;
        GetData(NewResult.Radius, Status.MPI_SOURCE);
        GetData(NewResult.Offset, Status.MPI_SOURCE);
        GetData(NewResult.EdgeIndex, Status.MPI_SOURCE);
        if (NewResult.Radius < BestResult.Radius) {
            BestResult = NewResult;
        }
    }
    return BestResult;
}
*/
void  IMpiHelper::SendCmd(unsigned Cmd, int DestTask)
{
    MPI_Send(&Cmd, 1, MPI_UNSIGNED, DestTask, MT_COMMANDS, MPI_COMM_WORLD);
}

ECommands  IMpiHelper::RecieveCmd(int SourceTask, MPI_Status* pStatus)
{
    unsigned Cmd;
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&Cmd, 1, MPI_INT, SourceTask, MT_COMMANDS, MPI_COMM_WORLD, pStatus);
    return static_cast<ECommands>(Cmd);
}

void  IMpiHelper::RecieveAndCheckCmd(ECommands Cmd, int SourceTask, MPI_Status* pStatus)
{
    ECommands GotCmd = RecieveCmd(SourceTask, pStatus);
    if (Cmd != GotCmd) {
        ThrowBadCmd(Cmd, GotCmd);
    }
}

void  IMpiHelper::GetData(unsigned& Result, int SourceTask, MPI_Status* pStatus)
{
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&Result, 1, MPI_UNSIGNED, SourceTask, MT_DATA_PARAMS, MPI_COMM_WORLD, pStatus);
}

void  IMpiHelper::GetData(double& Result, int SourceTask, MPI_Status* pStatus)
{
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&Result, 1, MPI_DOUBLE, SourceTask, MT_DATA_PARAMS, MPI_COMM_WORLD, pStatus);
}

void  IMpiHelper::SendData(unsigned Data, int DestTask)
{
    MPI_Send(&Data, 1, MPI_UNSIGNED, DestTask, MT_DATA_PARAMS, MPI_COMM_WORLD);
}

void  IMpiHelper::SendData(double Data, int DestTask)
{
    MPI_Send(&Data, 1, MPI_DOUBLE, DestTask, MT_DATA_PARAMS, MPI_COMM_WORLD);
}

void  IMpiHelper::GetDataBcast(unsigned& Result)
{
    assert( false == IsSupervisor() );
    MPI_Bcast(&Result, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
}

void  IMpiHelper::SendDataBcast(unsigned Data)
{
    assert( IsSupervisor() );
    MPI_Bcast(&Data, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
}

void  IMpiHelper::ThrowBadCmd(ECommands ExpectedCmd, ECommands GotCmd)
{
    std::stringstream ss;
    ss << "Received wrond cmd. Wait " << CmdToStr[ExpectedCmd] <<
          ". Got" << CmdToStr[GotCmd];
    throw ss.str().c_str();
}

/* Logging */

std::ostream& operator<<(std::ostream& out, CLogedItem LogedItem)
{
    out << LogedItem.msg << " (" << LogedItem.startTimeMs << "ms - " << LogedItem.finishTimeMs << "ms: " <<
           LogedItem.finishTimeMs - LogedItem.startTimeMs << "ms)\n";
    return out;
}

MixTaskLogger::MixTaskLogger(int ownerRank, std::string name):
    m_ownerRank(ownerRank),
    m_name(name)
{}

void MixTaskLogger::PublishLog()
{
    char fname[30];
    std::ofstream ofile;
    sprintf(fname, "task%02d.txt", m_ownerRank);
    ofile.open(fname);
    PublishLog(ofile);
    ofile.close();
}

void MixTaskLogger::PublishItem(std::ostream& out, CLogedItem* li)
{
    out << "[" << m_ownerRank << "]" << m_name << " " << *li;
}

void MixTaskLogger::PublishLog(std::ostream& out)
{
    for (std::list<CLogedItem*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it) {
        PublishItem(out, *it);
    }
}

void MixTaskLogger::SendLog()
{
    throw("Not implemented"); // TODO: implement
}

void MixTaskLogger::PutToLog(CLogedItem* loggedItem)
{
#ifdef STDERR_LOG_DUMP
    PublishItem(std::cerr, loggedItem);
#endif
    m_buffer.push_back(loggedItem);
}

CLogHelper::CLogHelper(ILogger& log, std::string msg)
{
    m_logedItem = new CLogedItem;
    m_logedItem->startTimeMs = clock()*1000 / CLOCKS_PER_SEC;
    m_logedItem->finishTimeMs = m_logedItem->startTimeMs;
    m_logedItem->msg = msg;
    log.PutToLog(m_logedItem);
}

CLogHelper::~CLogHelper()
{
    m_logedItem->finishTimeMs = clock()*1000 / CLOCKS_PER_SEC;
}
