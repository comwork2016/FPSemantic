#include "RangeUtil.h"

RangeUtil::RangeUtil()
{
    //ctor
}

int RangeUtil::MergeRangeToVector(std::vector<PAIRDOCRANGETIMES>& vec_DocRangeTimes,PAIRDOCRANGETIMES& docRangeTimes,int indexBegin)
{
    int n_Merge = -1;
    bool b_MergeRange = false;
    Range range = docRangeTimes.first;
    int times = docRangeTimes.second;
    //std::cout<<std::endl<<"["<<range.begin<<","<<range.end<<"]\t"<<times<<std::endl;
    //std::cout<<"************************"<<std::endl;
    //遍历已保存的所有位置
    for(int i=indexBegin; i < vec_DocRangeTimes.size(); i++)
    {
        PAIRDOCRANGETIMES docRangeTimes2 = vec_DocRangeTimes[i];
        Range range2 = docRangeTimes2.first;
        int times2 = docRangeTimes2.second;
        //std::cout<<"["<<range2.begin<<","<<range2.end<<"]\t"<<times2<<"\t"<<std::endl;
        //四种情况可以合并，包含，交叉各两种
        //交叉1 range:【6，13】和range2【10，24】
        if(range.begin <= range2.begin && range.end >= range2.begin - SIMILARRANGE && range.end <= range2.end)
        {
            range.end = range2.end;
            b_MergeRange = true;
        }
        //包含1 range:【6，28】和range2【10，24】
        else if(range.begin <= range2.begin && range.end >= range2.end)
        {
            //范围不变
            b_MergeRange = true;
        }
        //包含2 range:【16，18】和range2【10，24】
        else if(range.begin >= range2.begin && range.end <= range2.end)
        {
            range.begin = range2.begin;
            range.end = range2.end;
            b_MergeRange = true;
        }
        //交叉2 range:【22，25】和range2【10，24】
        else if(range.begin >= range2.begin && range.begin <= range2.end + SIMILARRANGE )
        {
            range.begin = range2.begin;
            b_MergeRange = true;
        }
        if(b_MergeRange)//能够合并，删除旧范围，返回
        {
            //更新要添加的集合
            times = times + times2;
            docRangeTimes.first = range;
            docRangeTimes.second = times;
            //删除旧集合中的元素
            vec_DocRangeTimes.erase(vec_DocRangeTimes.begin()+i);
            n_Merge = i;
            break;
        }
    }
    if(!b_MergeRange)//范围位置偏差太大，则新建一个位置存放词语信息
    {
        vec_DocRangeTimes.push_back(docRangeTimes);
    }
    return n_Merge;
}

std::vector<PAIRSENRANGE> RangeUtil::MergeRangeInSentence(std::vector<PAIRSIMWORDNO>& vec_SimWordNo)
{
    std::vector<PAIRDOCRANGETIMES> vec_DocRangeTimes1,vec_DocRangeTimes2;
    for(int i=0; i<vec_SimWordNo.size(); i++)
    {
        PAIRSIMWORDNO pair_SimWordNo = vec_SimWordNo[i];
        Range range1,range2;
        range1.begin = pair_SimWordNo.first;
        range1.end = pair_SimWordNo.first;
        PAIRDOCRANGETIMES docRangeTimes1(range1,1);
        range2.begin = pair_SimWordNo.second;
        range2.end = pair_SimWordNo.second;
        PAIRDOCRANGETIMES docRangeTimes2(range2,1);
        std::vector<PAIRDOCRANGETIMES> vec_TmpDocRangeTimes1 = vec_DocRangeTimes1;//副本
        std::vector<PAIRDOCRANGETIMES> vec_TmpDocRangeTimes2 = vec_DocRangeTimes2;
        PAIRDOCRANGETIMES tmp_docRangeTimes1 = docRangeTimes1;
        PAIRDOCRANGETIMES tmp_docRangeTimes2 = docRangeTimes2;
        int n_Merge1 = MergeRangeToVector(vec_DocRangeTimes1,docRangeTimes1);
        int n_Merge2 = MergeRangeToVector(vec_DocRangeTimes2,docRangeTimes2);
        /*
            [0,8]7------[0,7]7
            [14,14]1------[8,8]1
            [12,15]2------[18,20]2

            要合并17,9时不能正确合并
        */
        while(n_Merge1!=-1 && n_Merge2 !=-1)
        {
            if(n_Merge1 == n_Merge2)//都存在且合并的位置相对应
            {
                vec_TmpDocRangeTimes1 = vec_DocRangeTimes1;
                vec_TmpDocRangeTimes2 = vec_DocRangeTimes2;
                tmp_docRangeTimes1 = docRangeTimes1;
                tmp_docRangeTimes2 = docRangeTimes2;
                n_Merge1 = MergeRangeToVector(vec_DocRangeTimes1,docRangeTimes1);
                n_Merge2 = MergeRangeToVector(vec_DocRangeTimes2,docRangeTimes2);
            }
            else//否则还原并向下查找
            {
                vec_DocRangeTimes1 = vec_TmpDocRangeTimes1;
                vec_DocRangeTimes2 = vec_TmpDocRangeTimes2;
                docRangeTimes1 = tmp_docRangeTimes1;
                docRangeTimes2 = tmp_docRangeTimes2;
                if(n_Merge1 < n_Merge2) //如果n_Merge1小，则从n_Merge1 + 1处继续查找
                {
                    n_Merge1 = MergeRangeToVector(vec_DocRangeTimes1,docRangeTimes1,n_Merge1+1);
                    n_Merge2 = MergeRangeToVector(vec_DocRangeTimes2,docRangeTimes2,n_Merge2);
                }
                else
                {
                    n_Merge1 = MergeRangeToVector(vec_DocRangeTimes1,docRangeTimes1,n_Merge1);
                    n_Merge2 = MergeRangeToVector(vec_DocRangeTimes2,docRangeTimes2,n_Merge2+1);
                }
            }
        }
        if(n_Merge1== -1 || n_Merge2 || -1 || n_Merge1 !=n_Merge2)//如果不能合并，则还原，并且加到结尾。
        {
            vec_DocRangeTimes1 = vec_TmpDocRangeTimes1;
            vec_DocRangeTimes2 = vec_TmpDocRangeTimes2;
            vec_DocRangeTimes1.push_back(tmp_docRangeTimes1);
            vec_DocRangeTimes2.push_back(tmp_docRangeTimes2);
        }
    }
    std::vector<PAIRSENRANGE> vec_PairSenRange;
    for(int m=0; m<vec_DocRangeTimes1.size(); m++)
    {
        PAIRDOCRANGETIMES docRangeTimes1 = vec_DocRangeTimes1[m];
        PAIRDOCRANGETIMES docRangeTimes2 = vec_DocRangeTimes2[m];
        if(docRangeTimes1.second >= KGRAM && docRangeTimes2.second >= KGRAM)
        {
            Range range1 = docRangeTimes1.first;
            Range range2 = docRangeTimes2.first;
            PAIRSENRANGE pair_Range(range1,range2);
            vec_PairSenRange.push_back(pair_Range);
            //std::cout<<"["<<range1.begin<<","<<range1.end<<"]"<<docRangeTimes1.second<<"------["<<range2.begin<<","<<range2.end<<"]"<<docRangeTimes2.second<<std::endl;
        }
    }
    return vec_PairSenRange;
}

RangeUtil::~RangeUtil()
{
    //dtor
}
