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
        std::vector<SimilarDocRange> vec_SimilarDocRange = docDao->GetSentenceSimilarDocument(doc);
        for(int i=0; i<vec_SimilarDocRange.size(); i++)
        {
            SimilarDocRange similarDocRange = vec_SimilarDocRange[i];
            std::cout<<similarDocRange.docID_DB<<"\t["<<similarDocRange.textrange_SimilarDoc.offset<<","<<similarDocRange.textrange_SimilarDoc.length<<"]"<<std::endl;
            std::cout<<similarDocRange.str_Similar<<std::endl;
            std::cout<<similarDocRange.str_Search<<std::endl;
            std::cout<<similarDocRange.similarity<<std::endl;
        }
        /*遍历输出相同指纹*/
        /*for(int i=0; i<vec_SimilarDocument.size(); i++)
        {
            FingerPrintsSimilarDocument similarDoc = vec_SimilarDocument[i];
            const char* pch_DocPath = doc->GetstrDocPath().c_str();
            const char* pch_SimDocPath = similarDoc.str_DBDoc.c_str();
            std::cout<<std::endl<<std::endl<<"*************************************************************************************************"<<std::endl;
            std::cout<<"similarity between "<<pch_DocPath<<" and "<<pch_SimDocPath<<" is "<<100*similarDoc.f_similarity<<"%"<<std::endl;
            std::cout<<"*************************************************************************************************"<<std::endl;
            int n_SameSize = similarDoc.vec_SearchDocSimilarTextRange.size();
            for(int j=0; j<n_SameSize; j++)
            {
                std::cout<<"==============================="<<std::endl;
                TextRange textrange_SearchDoc = similarDoc.vec_SearchDocSimilarTextRange[j]; //待比对文档中相同的hash
                TextRange textrange_DBDoc = similarDoc.vec_DBDocSimilarTextRange[j]; //数据库中文档的相同的hash;
                // 搜索文档的内容和位置
                int n_OriginLength = textrange_SearchDoc.offset_end - textrange_SearchDoc.offset_begin;
                std::string str_OriginWord = doc->GetstrContents().substr(textrange_SearchDoc.offset_begin, n_OriginLength);
                std::cout<<"["<<textrange_SearchDoc.offset_begin<<","<<textrange_SearchDoc.offset_end<<","<<n_OriginLength<<"]"<<std::endl;
                std::cout<<str_OriginWord<<std::endl<<std::endl;
                //数据库中文档的内容和位置
                Document* docDB = new Document(similarDoc.str_DBDoc);// 数据库中的文档信息
                int n_DBLength = textrange_DBDoc.offset_end - textrange_DBDoc.offset_begin;
                std::string str_DBWord = docDB->GetstrContents().substr(textrange_DBDoc.offset_begin, n_DBLength);
                std::cout<<"["<<textrange_DBDoc.offset_begin<<","<<textrange_DBDoc.offset_end<<","<<n_DBLength<<"]"<<std::endl;
                std::cout<<str_DBWord<<std::endl;
            }
        }*/
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
