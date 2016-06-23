#ifndef RANGEUTIL_H
#define RANGEUTIL_H

#include "DataStructure.h"
#include <iostream>
#include "DataStructure.h"
#include <map>

class RangeUtil
{
    public:
        RangeUtil();
        virtual ~RangeUtil();

        static int MergeRangeToVector(std::vector<PairRangeTimes>& vec_DocRangeTimes,PairRangeTimes& docRangeTimes,int indexBegin = 0);
        static std::vector<PairSenRange> MergeRangeInSentence(std::vector<PairSimWordNo>& vec_SimWordNo);
        static void MergeLongestSimilarSentence(std::vector<SimilarDoc>& vec_SimilarDocForSen, SimilarDoc& similarDoc);

    protected:
    private:
};

#endif // RANGEUTIL_H
