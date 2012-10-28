#include "log.h"
#include <cstdio>
#include <fstream>

std::ostream& operator<<(std::ostream& out, TLogedItem LogedItem)
{
    out << LogedItem.Msg << " (" << LogedItem.StartTimeMs << "ms - " << LogedItem.FinishTimeMs << "ms: " <<
           LogedItem.FinishTimeMs - LogedItem.StartTimeMs << "ms)\n";
    return out;
}

void TLog::PublishLog(std::ostream& out)
{
    for (std::list<TLogedItem*>::iterator it = Buffer.begin(); it != Buffer.end(); ++it) {
        out << **it;
    }
}

void TLog::SendLog()
{
    throw("Not implemented"); // TODO: implement
}

TLog& TLog::operator<<(TLogedItem* LoggedItem)
{
    Buffer.push_back(LoggedItem);
    return *this;
}

TLogHelper::TLogHelper(TLog& Log, std::string Msg)
{
    LogedItem = new TLogedItem;
    LogedItem->StartTimeMs = clock()*1000 / CLOCKS_PER_SEC;
    LogedItem->FinishTimeMs = LogedItem->StartTimeMs;
    LogedItem->Msg = Msg;

    Log << LogedItem;
}

TLogHelper::~TLogHelper()
{
    LogedItem->FinishTimeMs = clock()*1000 / CLOCKS_PER_SEC;
}

