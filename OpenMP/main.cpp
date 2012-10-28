#include "array.h"

int main()
{
    CSimpleArray arr(10);
    //    arr.FillDec();
        arr.FillInc();
    //arr.FillRand();
    std::cerr << arr << "\n";
    arr.InsertSort();
    std::cerr << arr << "\n";
    arr.GetLog().PublishLog(std::cerr);
    arr.ValidateSort();
    return 0;
}
