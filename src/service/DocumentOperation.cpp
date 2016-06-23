#include "DocumentOperation.h"

DocumentOperation::DocumentOperation()
{
    //ctor
}

/**
    添加一个文档
*/
int DocumentOperation::AddDocument(const std::string& str_DocPath)
{
    Document* doc = new Document(str_DocPath,true);
    //分词
    NLPIRUtil* nlpirUtil = new NLPIRUtil();
    nlpirUtil->SplitDocument(doc);
    delete nlpirUtil;
    //计算hash值
    doc->CalcDocSimHash();
    DocumentDao* docDao = new DocumentDao();
    std::vector<Document*> vec_Doc;
    vec_Doc.push_back(doc);
    docDao->InsertDocuments(vec_Doc);
    delete docDao;
    delete doc;
}

/**
    将整个目录中的文件添加到数据库中
*/
int DocumentOperation::AddDirectoryDocuments(const std::string& str_InputDir)
{
    DocumentDao* docDao = new DocumentDao();
    docDao->DeleteAll();
    NLPIRUtil* nlpirUtil = new NLPIRUtil();
    std::vector<Document*> vec_Documents;
    //读取目录下所有的文件
    struct dirent *ptr;
    DIR *dir;
    dir=opendir((char *)str_InputDir.c_str());
    if(!dir)
    {
        std::cout<<"read input dir error"<<std::endl;
        return 1;
    }
    int num=0;
    while((ptr=readdir(dir))!=NULL)
    {
        //跳过'.'和'..'两个目录
        if(ptr->d_name[0] == '.' || ptr->d_name[strlen(ptr->d_name)-1] == '~')
        {
            continue;
        }
        std::string str_DocPath = str_InputDir + ptr->d_name;
        Document* doc = new Document(str_DocPath,true);
        nlpirUtil->SplitDocument(doc);
        doc->CalcDocSimHash();
        vec_Documents.push_back(doc);
        num++;
        //一次
        //if(num%10==0)
        if(num%1000==0)
        {
            num=0;
            //将文档集合添加到数据库中
            docDao->InsertDocuments(vec_Documents);
            //释放所有文档资源
            for(int i=0; i<vec_Documents.size(); i++)
            {
                delete vec_Documents[i];
            }
            vec_Documents.clear();
            //break;
        }
    }
    //将文档集合添加到数据库中
    docDao->InsertDocuments(vec_Documents);
    //释放所有文档资源
    for(int i=0; i<vec_Documents.size(); i++)
    {
        delete vec_Documents[i];
    }
    closedir(dir);
    delete nlpirUtil;
    delete docDao;
    return 0;
}

int DocumentOperation::SearchLeak(const std::string& str_DocPath)
{
    Document* doc = new Document(str_DocPath,true);
    NLPIRUtil* nlpirUtil = new NLPIRUtil();
    nlpirUtil->SplitDocument(doc);
    delete nlpirUtil;
    doc->CalcDocSimHash();
    //doc->SplitSentencesToKGrams();
    //doc->Display();
    //与数据库中的文件SimHash比较,如果不相同,再通过文档指纹查询泄露信息
    DocumentDao* docDao = new DocumentDao();
    std::string str_SimilarDoc = docDao->QuerySIMSimilarity(doc);
    if(str_SimilarDoc=="")
    {
        //查询相同的指纹
        std::vector<SimilarDoc> vec_SimilarDoc = docDao->GetSentenceSimilarDocument(doc);
        for(int i=0; i<vec_SimilarDoc.size(); i++)
        {
            SimilarDoc similarDoc = vec_SimilarDoc[i];
            std::cout<<"*************************************************************************************************"<<std::endl;
            std::cout<<"["<<similarDoc.textrange_SearchDoc.offset<<","<<similarDoc.textrange_SearchDoc.length<<"]"<<std::endl;
            std::cout<<similarDoc.str_Search<<std::endl<<std::endl<<std::endl;

            std::cout<<similarDoc.docID_DB<<"\t["<<similarDoc.textrange_SimilarDoc.offset<<","<<similarDoc.textrange_SimilarDoc.length<<"]"<<std::endl;
            std::cout<<similarDoc.str_Similar<<std::endl;
            std::cout<<"similarity is "<<similarDoc.similarity<<std::endl;
        }
    }
    else
    {
        const char* pch_DocName =doc->GetstrDocName().c_str();
        const char* pch_SimilarDocName =str_SimilarDoc.c_str();
        std::cout<<"LEAKAGE DOC FOUND: "<<pch_DocName <<" is similar to "<<pch_SimilarDocName<<std::endl;
    }
    delete docDao;
    delete doc;
    return 0;
}

DocumentOperation::~DocumentOperation()
{
    //dtor
}
