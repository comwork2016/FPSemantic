#include <iostream>
#include <ctime>

#include "Document.h"
#include "ReadCorpus.h"
#include "DocumentOperation.h"
#include "GlossaryDao.h"
//to delete
#include "SentenceSimilarity.h"
#include "LongestSimilarSentence.h"

//静态变量的初始化
std::map<std::string,double> ReadCorpus::map_CorpusTF;
std::set<std::string> ReadCorpus::set_StopTerm;
std::vector<Sememe> ReadCorpus::vec_Sememe;

int main()
{
    //设置全局环境本地化
    std::locale::global(std::locale(""));
    struct timeval start,finish;
    double duration;
    gettimeofday(&start,NULL);

    /*//将词语义项存入数据库中
    GlossaryDao* glossaryDao = new GlossaryDao();
    std::string str_GlossaryPath="./dat/glossary.dat";
    glossaryDao->ReadGlossaryToDB(str_GlossaryPath);
    delete glossaryDao;*/

    //读取语料库中的词频信息
    //ReadCorpus::ReadCorpusTF("./Corpus/Corpus.csv");
    ReadCorpus::ReadStopTerm("./Corpus/StopTerm.txt");
    ReadCorpus::ReadSememe("./dat/whole.dat");

    //DocumentOperation::AddDocument("./in/utf_23.txt");
    std::string str_InputDir = "./in/";
    //DocumentOperation::AddDirectoryDocuments(str_InputDir);

    //查询数据泄露
    //std::string str_LeakDoc = "./in/utf_1228.txt";
    std::string str_LeakDoc = "./test/leak.txt";
    DocumentOperation::SearchLeak(str_LeakDoc);

    //Document* doc = new Document("./in/utf_742.txt",true);
    //doc->Display();

/*
    LongestSimilarSentence* lss = new LongestSimilarSentence();
    std::string str_Search = "全要素之间练协同、模块之间练集成的方法，探索";
    std::string str_Similar = "练集成，积极稳妥地探索实践一体化训练的方法";
    std::vector<PAIRSENRANGE> vec_PairSenRange;
    double d_similarity = lss->GetSimBoundary(str_Search,str_Similar,vec_PairSenRange);
    std::cout<<d_similarity<<std::endl;
    delete lss;
    for(int x=0; x<vec_PairSenRange.size(); x++)
    {
        PAIRSENRANGE pair_SenRange = vec_PairSenRange[x];
        //leakDoc
        Range range_SeachSen = pair_SenRange.first;
        // db doc
        Range range_SimSen = pair_SenRange.second;
        std::cout<<range_SeachSen.begin<<","<<range_SeachSen.end<<"----------"<<range_SimSen.begin<<","<<range_SimSen.end<<std::endl;
    }

    SentenceSimilarity* ss = new SentenceSimilarity();
    std::cout<<ss->CalcSentenceSimilarity(str_Search,str_Similar)<<std::endl;
    delete ss;*/

    /*
    SentenceSimilarity* ss = new SentenceSimilarity();
    std::string str11 = "“龙眼”无人机是由美国海军陆战队士兵在阿富汗地区发射";
    std::string str12 = "海军陆战队战士在阿富汗山区发射“龙眼”无人机";
    std::cout<<ss->CalcSentenceSimilarity(str11,str12)<<std::endl;
    //相似度为0.48
    std::string str21 = "他们结合海军某快反支队海上训练的保障任务";
    std::string str22 = "训练由静态训练向动态训练转变，由室内训练向室外训练";
    std::cout<<ss->CalcSentenceSimilarity(str21,str22)<<std::endl;
    delete ss;*/

    gettimeofday(&finish,NULL);
    duration = 1000000 * (finish.tv_sec - start.tv_sec) + finish.tv_usec - start.tv_usec;
    duration /= 1000000;
    std::cout<<std::endl<<std::endl<<"cost "<<duration<<" s"<<std::endl<<std::endl;
    return 0;
}
