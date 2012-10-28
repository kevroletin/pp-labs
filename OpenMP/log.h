#ifndef LOG_H
#define LOG_H

#include <list>
#include <string>
#include <ostream>


struct TLogedItem {
    unsigned long long StartTimeMs;
    unsigned long long FinishTimeMs;
    std::string Msg;
};

std::ostream& operator<<(std::ostream& out, TLogedItem LogedItem);

class TLog {
public:
public:
    void PublishLog(std::string FilenamePrefix = "");
    void PublishLog(std::ostream& out);
    void SendLog();
    TLog& operator<<(TLogedItem* LoggedItem);

protected:
    std::list<TLogedItem*> Buffer;
};

class TLogHelper {
public:
    TLogHelper(TLog& Log, std::string Msg);
    ~TLogHelper();

protected:
    TLogedItem* LogedItem;
};

#endif // LOG_H
