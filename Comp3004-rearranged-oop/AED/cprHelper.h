#ifndef CPRHELPER_H
#define CPRHELPER_H

#include "defaultHeader.h"

class CprHelper{
public:
    void setCPRDepth(int);
    void performCpr();

private:
    int cprCount;
    int previourCpr;
};

#endif // CPRHELPER_H
