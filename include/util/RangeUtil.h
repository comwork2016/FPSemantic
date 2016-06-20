#ifndef RANGEUTIL_H
#define RANGEUTIL_H

#include "DataStructure.h"
#include <iostream>

class RangeUtil
{
    public:
        RangeUtil();
        virtual ~RangeUtil();

        static bool MergeRangeToVector(std::vector<PAIRDOCRANGETIMES>& vec_DocRangeTimes,PAIRDOCRANGETIMES& docRangeTimes);

    protected:
    private:
};

#endif // RANGEUTIL_H
