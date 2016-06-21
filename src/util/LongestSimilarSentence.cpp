#include "LongestSimilarSentence.h"

LongestSimilarSentence::LongestSimilarSentence()
{
    //ctor
    glossaryDao = new GlossaryDao();
}

double LongestSimilarSentence::GetSimBoundary(const std::string str_Sen1,const std::string str_Sen2,std::vector<PAIRSENRANGE>& vec_PairSenRange)
{
    //如果两个句子的长度相差一杯，一般是不相似的
    int len1 = str_Sen1.length();
    int len2 = str_Sen2.length();
    std::vector<SplitedHits> vec_Word1, vec_Word2;
    NLPIRUtil* nlpirUtil = new NLPIRUtil();
    vec_Word1 = nlpirUtil->SplitStringToWords(str_Sen1);
    vec_Word2 = nlpirUtil->SplitStringToWords(str_Sen2);
    delete nlpirUtil;
    //便利所有词语，提取词语概念
    std::vector<std::string> vec_StrWord1;
    std::vector<std::string> vec_StrWord2;
    std::map<std::string,std::vector<std::string> > map_WordConceptsVector;
    for(int i=0; i<vec_Word1.size(); i++)
    {
        std::string str_Word = vec_Word1[i].word;
        vec_StrWord1.push_back(str_Word);
        if(map_WordConceptsVector.find(str_Word) == map_WordConceptsVector.end())
        {
            std::vector<std::string> vec1 = glossaryDao->SelectSememe(str_Word);
            map_WordConceptsVector[str_Word] = vec1;
        }
    }
    for(int i=0; i<vec_Word2.size(); i++)
    {
        std::string str_Word = vec_Word2[i].word;
        vec_StrWord2.push_back(str_Word);
        if(map_WordConceptsVector.find(str_Word) == map_WordConceptsVector.end())
        {
            std::vector<std::string> vec2 = glossaryDao->SelectSememe(str_Word);
            map_WordConceptsVector[str_Word] = vec2;
        }
    }
    std::vector<PAIRSIMWORDNO> vec_SimWordNo;
    double d_sim = CalcVectorSimilarity(vec_StrWord1,vec_StrWord2,map_WordConceptsVector,vec_SimWordNo);
    vec_PairSenRange = RangeUtil::MergeRangeInSentence(vec_SimWordNo);
    return d_sim;
}

/**
    计算两个集合中词语的平均相似度
*/
double LongestSimilarSentence::CalcVectorSimilarity(std::vector<std::string>& vec1,std::vector<std::string>& vec2, std::map<std::string,std::vector<std::string> >& map_WordConceptsVector,std::vector<PAIRSIMWORDNO>& vec_SimWordNo)
{
    std::vector<double> vec_maxsim;
    int len1=vec1.size();
    int len2=vec2.size();
    //计算由两个向量组成的矩阵中的相似度
    double** matrix;
    //矩阵初始化
    matrix = new double*[len1];
    for(int i=0; i<len1; i++)
    {
        matrix[i] = new double[len2];
    }
    //计算矩阵中的词语相似度
    for(int i=0; i<len1; i++)
    {
        //std::cout<<vec1[i];
        //std::cout<<std::endl;
        for(int j=0; j<len2; j++)
        {
            //std::cout<<vec2[j]<<"\t";
            if(vec1[i] == vec2[j])
            {
                matrix[i][j] = 1;
            }
            else
            {
                std::vector<std::string> vec_concept1 = map_WordConceptsVector[vec1[i]];
                if(len1==0)
                {
                    std::cout<<"word1 "<<vec1[i]<<" not in dict"<<std::endl;
                    matrix[i][j] = 0;
                }
                else
                {
                    std::vector<std::string> vec_concept2 = map_WordConceptsVector[vec2[j]];
                    if(len2==0)
                    {
                        std::cout<<"word2 "<<vec2[j]<<" not in dict"<<std::endl;
                        matrix[i][j] = 0;
                    }
                    else
                    {
                        matrix[i][j] = WordSimilarity::CalcWordSimilarityByConcepts(vec_concept1,vec_concept2);
                    }
                }
            }
            //std::cout<<matrix[i][j]<<"\t";
        }
    }
    //查找矩阵中最大相似度最大的词语对，加入相似度向量中
    int num = 0;
    while(num<len1 && num <len2)
    {
        double max_sim = -1;
        int m,n;
        for(int i=0; i<len1; i++)
        {
            for(int j=0; j<len2; j++)
            {
                if(matrix[i][j]>max_sim && matrix[i][j] >0)
                {
                    max_sim = matrix[i][j];
                    m = i;
                    n = j;
                }
            }
        }
        if(max_sim < 0.6)//没有词语相似了
        {
            break;
        }
        //从向量中删除以计算的词语
        for(int i=0; i<len1; i++) //列向置为-1
        {
            matrix[i][n] = -1;
        }
        for(int i=0; i<len2; i++)
        {
            matrix[m][i] = -1;
        }
        vec_maxsim.push_back(max_sim);
        PAIRSIMWORDNO pair_SimWordNo(m,n);
        vec_SimWordNo.push_back(pair_SimWordNo);
        num++;
    }
    //释放matrix矩阵资源
    for(int i=0; i<len1; i++)
    {
        delete[] matrix[i];
    }
    delete[] matrix;
    //计算相似度
    double sim_sum = 0;
    for(int i=0;i<vec_maxsim.size();i++)
    {
        sim_sum += vec_maxsim[i];
    }
    return sim_sum/vec_maxsim.size();
}

LongestSimilarSentence::~LongestSimilarSentence()
{
    //dtor
    delete glossaryDao;
}
