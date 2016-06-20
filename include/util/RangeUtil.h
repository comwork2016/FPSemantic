#ifndef RANGEUTIL_H
#define RANGEUTIL_H

#include "DataStructure.h"
#include <iostream>
#include "DataStructure.h"

class RangeUtil
{
    public:
        RangeUtil();
        virtual ~RangeUtil();

        static int MergeRangeToVector(std::vector<PAIRDOCRANGETIMES>& vec_DocRangeTimes,PAIRDOCRANGETIMES& docRangeTimes,int indexBegin = 0);
        static std::vector<PAIRSENRANGE> MergeRangeInSentence(std::vector<PAIRSIMWORDNO>& vec_SimWordNo);

    protected:
    private:
};

#endif // RANGEUTIL_H
