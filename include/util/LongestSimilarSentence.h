#ifndef LONGESTSIMILARSENTENCE_H
#define LONGESTSIMILARSENTENCE_H

#include <string>
#include <vector>

#include "WordSimilarity.h"
#include "NLPIR.h"
#include "NLPIRUtil.h"
#include "GlossaryDao.h"
#include "RangeUtil.h"

class LongestSimilarSentence
{
    public:
        LongestSimilarSentence();
        virtual ~LongestSimilarSentence();
        std::vector<PAIRSENRANGE> GetSimBoundary(const std::string str_Sen1,const std::string str_Sen2);
    protected:
        std::vector<PAIRSIMWORDNO> CalcVectorSimilarity(std::vector<std::string>& vec1, std::vector<std::string>& vec2, std::map<std::string,std::vector<std::string> >& map_WordConceptsVector);
    private:
        GlossaryDao* glossaryDao;
};

#endif // LONGESTSIMILARSENTENCE_H
