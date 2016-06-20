#ifndef LONGESTSIMILARSENTENCE_H
#define LONGESTSIMILARSENTENCE_H

#include "GlossaryDao.h"

class LongestSimilarSentence
{
    public:
        LongestSimilarSentence();
        virtual ~LongestSimilarSentence();
    protected:
    private:
        GlossaryDao* glossaryDao;
};

#endif // LONGESTSIMILARSENTENCE_H
