#include "LongestSimilarSentence.h"

LongestSimilarSentence::LongestSimilarSentence()
{
    //ctor
    glossaryDao = new GlossaryDao();
}



LongestSimilarSentence::~LongestSimilarSentence()
{
    //dtor
    delete glossaryDao;
}
