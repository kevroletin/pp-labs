#include "mpi_helpers.h"
#include "labyrinth.h"
#include "mpi.h"

#include <cassert> // TODO: throw errors enstead
#include <fstream>
#include <sstream>

#ifdef DEBUG_COMMUNICATION
#    define CommLog(msg) Log(msg)
#    define CommLogEx(msg) LogEx(msg)
#else
#    define CommLog(msg)
#    define CommLogEx(msg)
#endif

void CEnvironment::InitMPI(int CmdLineArgc, char** CmdLineArgv)
{
    MPI_Init(&CmdLineArgc, &CmdLineArgv);
    MPI_Comm_size(MPI_COMM_WORLD, &m_procCnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
}

/*
TEdgePoint TSupervisor::Receiveresults()
{
    SUPERVISOR_LOG("Recieve results");
    TEdgePoint Bestresult = { INF, INF, -1 };
    for (unsigned i = 0; i < Graph.GetEdgesCnt(); ++i) {
        MPI_Status Status;
        RecieveAndCheckCmd(CMD_SEND_ANS_MAIN, MPI_ANY_SOURCE, &Status);
        TEdgePoint Newresult;
        GetData(Newresult.Radius, Status.MPI_SOURCE);
        GetData(Newresult.Offset, Status.MPI_SOURCE);
        GetData(Newresult.EdgeIndex, Status.MPI_SOURCE);
        if (Newresult.Radius < Bestresult.Radius) {
            Bestresult = Newresult;
        }
    }
    return Bestresult;
}
*/
void MixMpiHelper::SendCmd(unsigned cmd, int destTask)
{
    CommLogEx("SendCmd " << cmdToStr[cmd] << " to " << destTask);
    MPI_Send(&cmd, 1, MPI_UNSIGNED, destTask, MT_COMMANDS, MPI_COMM_WORLD);
}

ECommands MixMpiHelper::RecieveCmd(int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("RecieveCmd from " << sourceTask);
    unsigned cmd;
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&cmd, 1, MPI_INT, sourceTask, MT_COMMANDS, MPI_COMM_WORLD, pStatus);
    CommLogEx("RecieveCmd " << cmdToStr[cmd] << " from " << pStatus->MPI_SOURCE);
    return static_cast<ECommands>(cmd);
}

void MixMpiHelper::RecieveAndCheckCmd(ECommands Cmd, int sourceTask, MPI_Status* pStatus)
{
    ECommands GotCmd = RecieveCmd(sourceTask, pStatus);
    if (Cmd != GotCmd) {
        ThrowBadCmd(Cmd, GotCmd);
    }
}

void MixMpiHelper::GetData(unsigned& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData uint from " << sourceTask);
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&result, 1, MPI_UNSIGNED, sourceTask, MT_DATA_PARAMS, MPI_COMM_WORLD, pStatus);
    CommLogEx("GetData " << result);
}

void MixMpiHelper::GetData(int& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData int from " << sourceTask);    
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&result, 1, MPI_INT, sourceTask, MT_DATA_PARAMS, MPI_COMM_WORLD, pStatus);
    CommLogEx("GetData " << result);
}

void MixMpiHelper::GetData(double& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData double from " << sourceTask);    
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }
    MPI_Recv(&result, 1, MPI_DOUBLE, sourceTask, MT_DATA_PARAMS, MPI_COMM_WORLD, pStatus);
    CommLogEx("GetData " << result);
}

void MixMpiHelper::GetData(CSquareField& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData CSquareField from " << sourceTask);
    GetData(result.GetType(), sourceTask, pStatus);
    GetData(result.GetData(), sourceTask, pStatus);
    assert(result.GetType().m_dim == result.GetData().m_dim);
    result.Resize(result.GetType().m_dim);
}

void MixMpiHelper::GetData(CMatrix& matr, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData CMatrix from " << sourceTask);
    MPI_Status StubStatus;
    if ( NULL == pStatus) {
        pStatus = &StubStatus;
    }

    GetData(matr.m_dim.m_x, sourceTask, pStatus);
    GetData(matr.m_dim.m_y, sourceTask, pStatus);
    { CommLogEx("GetData CMatrix got size " << matr.m_dim.m_x*matr.m_dim.m_y); }
    matr.Resize();
    MPI_Recv(&matr.m_data[0], matr.m_dim.m_x*matr.m_dim.m_y, MPI_UNSIGNED, sourceTask, MT_DATA_STREAM, MPI_COMM_WORLD, pStatus);
    { CommLog("GetData CMatrix got array"); }
}

void MixMpiHelper::GetData(CMpiConnections& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData CMpiConnections from " << sourceTask);    
    GetData(result.m_topRank,    sourceTask, pStatus);
    GetData(result.m_rightRank,  sourceTask, pStatus);
    GetData(result.m_bottomRank, sourceTask, pStatus);
    GetData(result.m_leftRank,   sourceTask, pStatus);
}

void MixMpiHelper::GetData(CSideCoord& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData CSideCoord from " << sourceTask);    
    GetData(result.m_offset, sourceTask, pStatus);
    uint tmp = 0;
    GetData(tmp, sourceTask, pStatus);
    result.m_side = static_cast<ESide>(tmp);
    CommLogEx("GetData " << result);
}

void MixMpiHelper::GetData(CPointCoord& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData CPointCoord from " << sourceTask);    
    GetData(result.m_x, sourceTask, pStatus);
    GetData(result.m_y, sourceTask, pStatus);
    CommLogEx("GetData " << result);
}

void MixMpiHelper::GetData(std::string& result, int sourceTask, MPI_Status* pStatus)
{
    CommLogEx("GetData std::string from " << sourceTask);
    int size;
    GetData(size, sourceTask, pStatus);
    result.resize(size);
    MPI_Recv(&result[0], size, MPI_CHAR, sourceTask, MT_DATA_STREAM, MPI_COMM_WORLD, pStatus);    
    CommLogEx("GetData " << result);
}

void MixMpiHelper::SendData(unsigned data, int destTask)
{
    CommLogEx("SendData uint " << data << " to " << destTask);
    MPI_Send(&data, 1, MPI_UNSIGNED, destTask, MT_DATA_PARAMS, MPI_COMM_WORLD);
}

void MixMpiHelper::SendData(int data, int destTask)
{
    CommLogEx("SendData int " << data << " to " << destTask);
    MPI_Send(&data, 1, MPI_INT, destTask, MT_DATA_PARAMS, MPI_COMM_WORLD);
}

void MixMpiHelper::SendData(double data, int destTask)
{
    CommLogEx("SendData double " << data << " to " << destTask);
    MPI_Send(&data, 1, MPI_DOUBLE, destTask, MT_DATA_PARAMS, MPI_COMM_WORLD);
}

void MixMpiHelper::SendData(CMpiConnections data, int destTask)
{
    CommLogEx("SendData connections to " << destTask);
    SendData(data.m_topRank, destTask);
    SendData(data.m_rightRank, destTask);
    SendData(data.m_bottomRank, destTask);
    SendData(data.m_leftRank, destTask);
}

void MixMpiHelper::SendData(CSquareField& field, int destTask)
{
    CommLogEx("SendData CSquareField to " << destTask );
    SendData(field.GetType(), destTask);
    SendData(field.GetData(), destTask);    
}

void MixMpiHelper::SendData(CMatrix matr, int destTask)
{
    CommLogEx("SendData CMatrix to " << destTask);
    SendData(matr.m_dim.m_x, destTask);
    SendData(matr.m_dim.m_y, destTask);
    CommLogEx("SendData CMatrix array " << matr.m_dim.m_x*matr.m_dim.m_y);
    MPI_Send(&matr.m_data[0], matr.m_dim.m_x*matr.m_dim.m_y, MPI_UNSIGNED, destTask, MT_DATA_STREAM, MPI_COMM_WORLD);
}

void MixMpiHelper::SendData(CSideCoord data, int destTask)
{
    CommLogEx("SendData CSideCoord to " << destTask);
    SendData(data.m_offset, destTask);
    uint tmp = data.m_side;
    SendData(tmp, destTask);
}

void MixMpiHelper::SendData(CPointCoord data, int destTask)
{
    CommLogEx("SendData CSideCoord to " << destTask);
    SendData(data.m_x, destTask);
    SendData(data.m_y, destTask);
}

void MixMpiHelper::SendData(std::string data, int destTask)
{
    CommLogEx("SendData std::string to " << destTask);
    SendData(data.size(), destTask);
    MPI_Send(&data[0], data.size(), MPI_CHAR, destTask, MT_DATA_STREAM, MPI_COMM_WORLD);    
}


void MixMpiHelper::GetDataBcast(unsigned& result)
{
    assert( false == IsSupervisor() );
    MPI_Bcast(&result, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
}

void MixMpiHelper::SendDataBcast(unsigned Data)
{
    assert( IsSupervisor() );
    MPI_Bcast(&Data, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
}

void MixMpiHelper::ThrowBadCmd(ECommands ExpectedCmd, ECommands GotCmd)
{
    std::stringstream ss;
    ss << "Received wrond cmd. Wait " << cmdToStr[ExpectedCmd] <<
          ". Got" << cmdToStr[GotCmd];
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
//    if (m_ownerRank == 0) {
        std::stringstream ss;
        PublishItem(ss, loggedItem);
        std::cerr << ss.str();
//    }
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

// ****

int moveDx[] = { 0, 1, 0, -1 };
int moveDy[] = { -1, 0, 1, 0 };
std::string sideStr[] = { "ETop", "ERight", "EBottom", "ELeft" };

ESide Invert( ESide side ) { 
    return static_cast<ESide>( (side + 2) % 4 );
}

std::ostream& operator<<(std::ostream& out, CPointCoord point) {
    out << "x: " << point.m_x << ", y:" << point.m_y;
    return out;
}

std::ostream& operator<<(std::ostream& out, CSideCoord coord) {
    out << "side: " << sideStr[coord.m_side] << ", offset:" << coord.m_offset;
    return out;
}


