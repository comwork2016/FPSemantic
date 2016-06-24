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
        //void GetSimBoundary(const std::string str_Sen1,const std::string str_Sen2,std::vector<SenRangeSimilarity>& vec_SenRangeSimilarity);
        void GetSimBoundary(const std::vector<SplitedHits> vec_SearchWords,const std::string str_Sen2,std::vector<SenRangeSimilarity>& vec_SenRangeSimilarity);
    protected:
        void CalcSimMatrix(std::vector<std::string>& vec1,std::vector<std::string>& vec2, std::map<std::string,std::vector<std::string> >& map_WordConceptsVector,double** matrix);
        std::vector<PairSimWordNo> GetPairedSimWordNo(double** matrix,int len1, int len2);
        double CalcSentenceSimilarity(double** matrix,Range range_row, Range range_col);
    private:
        GlossaryDao* glossaryDao;
};

#endif // LONGESTSIMILARSENTENCE_H
