#include "LongestSimilarSentence.h"

LongestSimilarSentence::LongestSimilarSentence()
{
    //ctor
    glossaryDao = new GlossaryDao();
}

double LongestSimilarSentence::GetSimBoundary(const std::string str_Sen1,const std::string str_Sen2,std::vector<SenRangeSimilarity>& vec_SenRangeSimilarity)
{
    //如果两个句子的长度相差一杯，一般是不相似的
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
    //计算词语相似度矩阵
    int len1=vec_Word1.size();
    int len2=vec_Word2.size();
    double** matrix;
    //矩阵初始化
    matrix = new double*[len1];
    for(int i=0; i<len1; i++)
    {
        matrix[i] = new double[len2];
    }
    CalcSimMatrix(vec_StrWord1,vec_StrWord2,map_WordConceptsVector,matrix);//计算词语的相似度矩阵
    //获取相似词语的配对信息
    std::vector<PairSimWordNo> vec_SimWordNo = GetPairedSimWordNo(matrix,len1,len2);
    std::vector<SenRangeSimilarity> vec_SenRangeSimilarityTmp = RangeUtil::MergeRangeInSentence(vec_SimWordNo);//合并配对信息
    //计算配对之后的句子相似度
    for(int k=0; k< vec_SenRangeSimilarityTmp.size(); k++)
    {
        SenRangeSimilarity senRangeSimilarity = vec_SenRangeSimilarityTmp[k];
        Range range_Search = senRangeSimilarity.range_Search;
        Range range_Similar = senRangeSimilarity.range_Similar;
        senRangeSimilarity.similarity = CalcSentenceSimilarity(matrix,range_Search,range_Similar);//计算两个范围内句子的相似度
        //std::cout<<senRangeSimilarity.similarity<<std::endl;
        if(senRangeSimilarity.similarity > SENSIMGATE)//如果相似度大于阈值，从最终结果中移除
        {
            vec_SenRangeSimilarity.push_back(senRangeSimilarity);
        }
    }
    //释放matrix矩阵资源
    for(int i=0; i<len1; i++)
    {
        delete[] matrix[i];
    }
    delete[] matrix;
}

/**
    根据相似度矩阵和范围，计算两个句子的相似度
*/
double LongestSimilarSentence::CalcSentenceSimilarity(double** matrix,Range range_row, Range range_col)
{
    //查找矩阵中最大相似度最大的词语对，加入相似度向量中
    int len1 = range_row.end - range_row.begin + 1;
    int len2 = range_col.end - range_col.begin + 1;
    int* n_rowflag = new int[len1];
    int* n_colflag = new int[len2];
    //配对标识初始化
    for(int i=0; i<len1; i++)
    {
        n_rowflag[i] = 0;
    }
    for(int j=0; j<len2; j++)
    {
        n_colflag[j] = 0;
    }
    int num = 0;
    std::vector<double> vec_maxsim;
    while(num<len1 && num <len2)
    {
        double max_sim = -1;
        int m,n;
        for(int i=range_row.begin; i<range_row.end+1; i++)
        {
            if(n_rowflag[i - range_row.begin] == 0)//行没有配对过
            {
                for(int j=range_col.begin; j<range_col.end+1; j++)
                {
                    if(n_colflag[j - range_col.begin] == 0)//列没有配对过
                    {
                        if(matrix[i][j]>max_sim)
                        {
                            max_sim = matrix[i][j];
                            m = i;
                            n = j;
                        }
                    }
                }
            }
        }
        if(max_sim == -1)//没有词语相似了
        {
            break;
        }
        n_rowflag[m-range_row.begin] = 1;
        n_colflag[n-range_col.begin] = 1;
        vec_maxsim.push_back(max_sim);
        num++;
    }
    delete[] n_rowflag;
    delete[] n_colflag;
    //对于剩下的词语，与空格计算相似度并加入到相似度向量中
    int n_rest = len1>len2?len1-num:len2-num;
    for(int i=0; i<n_rest; i++)
    {
        double d_NullSim = delta;
        vec_maxsim.push_back(d_NullSim);
    }
    //把整体相似度还原为部分相似度的加权平均,这里权值取一样，即计算算术平均
    if(vec_maxsim.size()==0)
    {
        return 0.0;
    }
    double sum=0.0;
    std::vector<double>::const_iterator it=vec_maxsim.begin();
    while(it!=vec_maxsim.end())
    {
        sum+=*it;
        it++;
    }
    return sum/vec_maxsim.size();
}

/**
    计算由两个向量组成的矩阵中的相似度
*/
void LongestSimilarSentence::CalcSimMatrix(std::vector<std::string>& vec1,std::vector<std::string>& vec2, std::map<std::string,std::vector<std::string> >& map_WordConceptsVector,double** matrix)
{
    int len1=vec1.size();
    int len2=vec2.size();
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
}

/**
    计算两个集合中配对的相似词语
*/
std::vector<PairSimWordNo> LongestSimilarSentence::GetPairedSimWordNo(double** matrix,int len1, int len2)
{
    std::vector<PairSimWordNo> vec_SimWordNo;
    //查找矩阵中最大相似度最大的词语对，加入相似度向量中
    int num = 0;
    int* n_rowflag = new int[len1];
    int* n_colflag = new int[len2];
    //配对标识初始化
    for(int i=0; i<len1; i++)
    {
        n_rowflag[i] = 0;
    }
    for(int j=0; j<len2; j++)
    {
        n_colflag[j] = 0;
    }
    while(num<len1 && num <len2)
    {
        double max_sim = -1;
        int m,n;
        for(int i=0; i<len1; i++)
        {
            if(n_rowflag[i] == 0)//行没有配对过
            {
                for(int j=0; j<len2; j++)
                {
                    if(n_colflag[j] == 0) //列没有配对过
                    {
                        if(matrix[i][j]>max_sim)
                        {
                            max_sim = matrix[i][j];
                            m = i;
                            n = j;
                        }
                    }
                }
            }
        }
        if(max_sim < WORDSIMGATE)//没有词语相似了
        {
            break;
        }
        PairSimWordNo pair_SimWordNo(m,n);
        vec_SimWordNo.push_back(pair_SimWordNo);
        n_rowflag[m] = 1;
        n_colflag[n] = 1;
        num++;
    }
    delete[] n_rowflag;
    delete[] n_colflag;
    return vec_SimWordNo;
}

LongestSimilarSentence::~LongestSimilarSentence()
{
    //dtor
    delete glossaryDao;
}
