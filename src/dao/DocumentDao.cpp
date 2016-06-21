#include "DocumentDao.h"

DocumentDao::DocumentDao()
{
    //ctor//ctor
    this->m_Host = "localhost:27017";
    this->m_DocDBName = "FP.docs";
    this->m_IndexDBName = "FP.wordindex";
    this->m_Conn.connect(this->m_Host);
}

/**
    查询数据库中的索引信息
    //登陆舰(n,9)—>{a.txt,tf, (124,247,513, 698, 1546, 1611, 1684,2018)}
*/
std::map<DOC_ID,WordIndexRecord*> DocumentDao::QueryIndexOfWord(const std::string& str_Word)
{
    std::map<DOC_ID,WordIndexRecord*> map_WordIndexRecord;
    mongo::Query query = QUERY("word"<<str_Word);
    mongo::BSONObj bo_columns = BSON("docs"<<1);
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->m_Conn.query(this->m_IndexDBName,query,0,0,&bo_columns);
    while (cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        mongo::BSONArray arrDoc(p.getObjectField("docs"));
        for(mongo::BSONObjIterator it= arrDoc.begin(); it.more();)
        {
            mongo::BSONElement be = it.next();
            mongo::BSONObj bson = be.Obj();
            std::string str_DocID = bson.getStringField("docid");
            WordIndexRecord* wordIndexRecord = new WordIndexRecord(str_DocID);
            mongo::BSONArray arrPos(bson.getObjectField("poss"));
            for(mongo::BSONObjIterator itr = arrPos.begin(); itr.more();)
            {
                mongo::BSONObj bson_Pos = itr.next().Obj();
                int n_WordPos = bson_Pos.getIntField("wordpos");
                int n_NoInDoc = bson_Pos.getIntField("noindoc");
                WordPos wordPos = {n_WordPos, n_NoInDoc};
                wordIndexRecord->AddPosInfo(wordPos);
            }
            map_WordIndexRecord[str_DocID] =wordIndexRecord;
        }
    }
    return map_WordIndexRecord;
}

bool DocumentDao::isWordIndexExists(const std::string& str_Word)
{
    mongo::Query query = QUERY("word"<<str_Word);
    mongo::BSONObj obj = this->m_Conn.findOne(this->m_IndexDBName,query);
    return obj.isEmpty()?false:true;
}

/**
    将文档索引添加到数据库中
*/
int DocumentDao::InsertIndexes(std::map<std::string,WordIndex*> map_WordIndex)
{
    std::cout<<"inserting indexes of docs"<<std::endl;
    for(std::map<std::string,WordIndex*>::iterator it = map_WordIndex.begin(); it != map_WordIndex.end(); it++)
    {
        std::string str_Word = it->first;
        WordIndex* wordIndex = it->second;
        std::map<DOC_ID,WordIndexRecord*> map_DocWordIndex = wordIndex->GetMapDocWordIndex();
        //构建BSON，用于更新数据库
        mongo::BSONArrayBuilder bb_DocsArray;
        for(std::map<DOC_ID,WordIndexRecord*>::iterator it = map_DocWordIndex.begin(); it != map_DocWordIndex.end(); it++)
        {
            DOC_ID docID = it->first;
            WordIndexRecord* record = it->second;
            mongo::BSONObjBuilder bb_Record;
            bb_Record.append("docid",record->GetDocID());
            bb_Record.appendNumber("tf",record->GetnTermFrequency());
            mongo::BSONArrayBuilder bb_PosArray;
            for(int j=0; j<record->GetVecPos().size(); j++)
            {
                WordPos wordPos = record->GetVecPos()[j];
                mongo::BSONObjBuilder bb_pos;
                bb_pos.append("wordpos",wordPos.wordPos);
                bb_pos.append("noindoc",wordPos.NoInDoc);
                bb_PosArray.append(bb_pos.obj());
            }
            bb_Record.append("poss",bb_PosArray.arr());
            bb_DocsArray.append(bb_Record.obj());
        }
        bool b_exists = isWordIndexExists(str_Word);
        if(!b_exists)//如果不存在，则新建索引并插入
        {
            mongo::BSONObjBuilder bb;
            bb.append("word",wordIndex->GetstrWord());
            bb.append("length",wordIndex->GetnWordLength());
            bb.append("POS",wordIndex->GetstrPOS());
            bb.append("docs",bb_DocsArray.arr());
            this->m_Conn.insert(this->m_IndexDBName,bb.obj());
        }
        else
        {
            //更新单词的文档索引
            mongo::BSONObjBuilder bb;
            mongo::BSONObjBuilder bb_Docs;
            bb_Docs.append("docs",bb_DocsArray.arr());
            bb.append("$pushAll",bb_Docs.obj());
            mongo::Query query = QUERY("word"<<str_Word);
            this->m_Conn.update(this->m_IndexDBName,query,bb.obj(),true,false);
        }
    }
    std::cout<<"insert indexes of docs done"<<std::endl;
    return 0;
}

/**
    插入一个文档到数据库中
*/
int DocumentDao::InsertDocument(Document* doc)
{
    mongo::BSONObjBuilder b;
    //保存文档信息
    b.append("docid", doc->GetDocID());
    b.append("filename",doc->GetstrDocName());
    b.append("filepath", doc->GetstrDocPath());
    const char* pch_Contents= doc->GetstrContents().c_str();
    b.append("filelength",static_cast<int>(StringUtil::ConvertCharArraytoWString(pch_Contents).length()));
    b.appendNumber("docsimhash",static_cast<long long>(doc->GetlSimHash()));
    b.appendNumber("docsimhash1",static_cast<long long>(doc->GetlSimHash16_1()));
    b.appendNumber("docsimhash2",static_cast<long long>(doc->GetlSimHash16_2()));
    b.appendNumber("docsimhash3",static_cast<long long>(doc->GetlSimHash16_3()));
    b.appendNumber("docsimhash4",static_cast<long long>(doc->GetlSimHash16_4()));
    /*
    //保存句子的范围信息
    mongo::BSONObjBuilder bb_Sens;
    for(int i=0;i<doc->GetvecParagraph().size();i++)
    {
        Paragraph para = doc->GetvecParagraph()[i];
        for(int j=0;j<para.vec_Sentences.size();j++)
        {
            Sentence sen = para.vec_Sentences[j];
            mongo::BSONObjBuilder bb_Range;
            std::string str_ParaSenNo;
            std::stringstream ss;
            ss<<i<<","<<j;
            bb_Range.append("senbegin",sen.textRange.offset);
            bb_Range.append("senlength",sen.textRange.length);
            bb_Sens.append(ss.str(),bb_Range.obj());
        }
    }
    b.append("senpos",bb_Sens.obj());*/
    this->m_Conn.insert(this->m_DocDBName,b.obj());
    std::cout<<doc->GetstrDocName() <<" inserted"<<std::endl;
    return 0;
}

/**
    插入一个文档集合到数据库中
*/
int DocumentDao::InsertDocuments(std::vector<Document*> vec_doc)
{
    for(int i=0; i<vec_doc.size(); i++)
    {
        Document* doc = vec_doc[i];
        const std::string str_SimilarDoc = QuerySIMSimilarity(doc);
        if(str_SimilarDoc=="")
        {
            doc->BuildInvertedIndex();
            InsertDocument(doc);
            std::cout<<doc->GetstrDocName() <<" inserted"<<std::endl;
        }
        else
        {
            std::cout<<"DUPLICATE DOC TO INSERT: "<<doc->GetstrDocName()<<" is similar to "<<str_SimilarDoc<<std::endl;
            //删除文档，不做插入操作
            vec_doc.erase(vec_doc.begin()+i);
            i--;
        }
    }
    std::cout<<"merging indexes of inserted docs"<<std::endl;
    //在更新索引前，先合并索引
    std::map<std::string,WordIndex*> map_WordIndexAll;
    for(int i=0; i<vec_doc.size(); i++)
    {
        Document* doc = vec_doc[i];
        std::map<std::string,WordIndex*> map_WordIndex = doc->GetMapWordIndex();
        //遍历文档中所有的词语
        for(std::map<std::string,WordIndex*>::iterator it = map_WordIndex.begin(); it != map_WordIndex.end(); it++)
        {
            std::string str_Word = it->first;//每个文档中的单词
            WordIndex* wordIndex = it->second;//每个词的索引
            //每个文件的倒排索引
            WordIndexRecord* record = wordIndex->GetMapDocWordIndex()[doc->GetDocID()];//特定文档中某个单词的倒排索引
            //将特定文档中的倒排索引合并到所有的倒排列表中
            WordIndex* wordIndexAll;
            if(map_WordIndexAll.find(str_Word)==map_WordIndexAll.end())//新增倒排列表中不存在该单词
            {
                wordIndexAll = wordIndex;
            }
            else
            {
                wordIndexAll = map_WordIndexAll[str_Word];
                //文档倒排列表添加到所有新增记录中
                std::map<DOC_ID,WordIndexRecord*> map_DocWordIndex = wordIndexAll->GetMapDocWordIndex();
                map_DocWordIndex[doc->GetDocID()] = record;
                wordIndexAll->SetMapDocWordIndex(map_DocWordIndex);
            }
            map_WordIndexAll[str_Word] = wordIndexAll;
        }
    }
    std::cout<<"merge indexes done"<<std::endl;
    InsertIndexes(map_WordIndexAll);
}

//删除数据库中的信息
int DocumentDao::DeleteAll()
{
    this->m_Conn.remove(this->m_IndexDBName,mongo::Query());
    this->m_Conn.remove(this->m_DocDBName,mongo::Query());
    return 0;
}

//从数据库中查询simhash值相似的文件名称，没有则返回""
std::string DocumentDao::QuerySIMSimilarity(const Document* doc)
{
    std::cout<<"Query similar simhash of document "<<doc->GetstrDocName()<<std::endl;
    std::string str_SimilarDoc = "";
    mongo::BSONObj bson_condition1 = BSON("docsimhash1"<<static_cast<long long>(doc->GetlSimHash16_1()));
    mongo::BSONObj bson_condition2 = BSON("docsimhash2"<<static_cast<long long>(doc->GetlSimHash16_2()));
    mongo::BSONObj bson_condition3 = BSON("docsimhash3"<<static_cast<long long>(doc->GetlSimHash16_3()));
    mongo::BSONObj bson_condition4 = BSON("docsimhash4"<<static_cast<long long>(doc->GetlSimHash16_4()));
    mongo::BSONObj bson_condition = mongo::OR(bson_condition1,bson_condition2,bson_condition3,bson_condition4);
    mongo::BSONObj bo_columns = BSON("docsimhash"<<1<<"filelength"<<1<<"filepath"<<1);
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->m_Conn.query(this->m_DocDBName,bson_condition,0,0,&bo_columns);
    while (cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        SIMHASH_TYPE l_SimHash = p.getField("docsimhash").numberLong();
        if(HashUtil::IsSimHashSimilar(doc->GetlSimHash(),l_SimHash))
        {
            //两个近似网页的文章长度差距应在20%以内
            int n_DBFileLength = p.getIntField("filelength");
            const char* pch_Contents= doc->GetstrContents().c_str();
            int n_DocFileLength = StringUtil::ConvertCharArraytoWString(pch_Contents).length();
            float f_LengthSim = (float)(n_DBFileLength-n_DocFileLength)/(n_DocFileLength>n_DBFileLength?n_DocFileLength:n_DBFileLength);
            if( f_LengthSim < 0.2)
            {
                str_SimilarDoc = std::string(p.getStringField("filepath"));
                break;
            }
        }
    }
    return str_SimilarDoc;
}

/**
查询句子相似的文档
*/
//std::vector<FingerPrintsSimilarDocument> DocumentDao::GetSentenceSimilarDocument(doc)
std::vector<SimilarDocRange> DocumentDao::GetSentenceSimilarDocument(const Document* doc)
{
    std::vector<SimilarDocRange> vec_SimilarDocRange;
    for(int i=0; i<doc->GetvecParagraph().size(); i++)
    {
        Paragraph para = doc->GetvecParagraph()[i];
        for(int j=0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            std::string str_Search = doc->GetstrContents().substr(sen.textRange.offset,sen.textRange.length);
            std::map<DOC_ID,int> map_DocOcurCount;
            //每一遍，统计文档出现的次数
            std::vector<std::map<DOC_ID,WordIndexRecord*> > vec_AllWordIndex;
            for(int k=0; k<sen.vec_splitedHits.size(); k++)
            {
                std::string str_Word = sen.vec_splitedHits[k].word;
                std::map<DOC_ID,WordIndexRecord*> map_WordDocIndexRecord = QueryIndexOfWord(str_Word);//单词索引的文档信息
                vec_AllWordIndex.push_back(map_WordDocIndexRecord);
                //遍历索引信息，统计文档出现的位置，能有效减少合并范围的个数
                for(std::map<DOC_ID,WordIndexRecord*>::iterator it = map_WordDocIndexRecord.begin(); it != map_WordDocIndexRecord.end(); it++)
                {
                    DOC_ID docID = it->first;
                    if(map_DocOcurCount.find(docID) == map_DocOcurCount.end())
                    {
                        map_DocOcurCount[docID] = 1;
                    }
                    else
                    {
                        map_DocOcurCount[docID] = map_DocOcurCount[docID]+1;
                    }
                }
            }
            //第二遍，统计相似
            //记录词语位置信息，并合并文档中的位置信息
            std::map<DOC_ID,std::map<int,Range> > map_DocWordNoPosition;//保存所有词语的位置信息。
            std::map<DOC_ID,std::vector<PAIRDOCRANGETIMES> > map_DocRangeVector; //单词范围的统计信息
            for(int k=0; k<vec_AllWordIndex.size(); k++)
            {
                std::string str_Word = sen.vec_splitedHits[k].word;
                std::map<DOC_ID,WordIndexRecord*> map_WordDocIndexRecord = vec_AllWordIndex[k];//单词索引的文档信息
                //遍历索引信息，保存词语位置信息，并保存生成DOC_ID，位置的次数关系
                for(std::map<DOC_ID,WordIndexRecord*>::iterator it = map_WordDocIndexRecord.begin(); it != map_WordDocIndexRecord.end(); it++)
                {
                    DOC_ID docID = it->first;
                    if(map_DocOcurCount[docID] < KGRAM)
                    {
                        continue;
                    }
                    WordIndexRecord* record = it->second;
                    std::map<int,Range> map_NoPositionInDoc = map_DocWordNoPosition[docID];//保存位置信息
                    //先对每个单词的位置进行合并
                    std::map<DOC_ID,std::vector<PAIRDOCRANGETIMES> > map_WordInDocRangeVector; //单词范围的统计信息
                    std::vector<WordPos> vec_WordPos = record->GetVecPos();
                    for(int m=0; m<vec_WordPos.size(); m++)
                    {
                        WordPos wordPos = vec_WordPos[m];
                        //保存词语的编号和位置的对应信息
                        Range posRange = {wordPos.wordPos, wordPos.wordPos+str_Word.length()};
                        map_NoPositionInDoc[wordPos.NoInDoc] = posRange;
                        Range wordRange = {wordPos.NoInDoc,wordPos.NoInDoc};
                        PAIRDOCRANGETIMES wordDocRangeTimes(wordRange,1);
                        if(map_WordInDocRangeVector.find(docID) == map_WordInDocRangeVector.end())
                        {
                            std::vector<PAIRDOCRANGETIMES> vec_WordDocRangeTimes;
                            vec_WordDocRangeTimes.push_back(wordDocRangeTimes);
                            map_WordInDocRangeVector[docID] = vec_WordDocRangeTimes;
                        }
                        else
                        {
                            std::vector<PAIRDOCRANGETIMES> vec_WordDocRangeTimes = map_WordInDocRangeVector[docID];//某个文档已保存的位置向量
                            while(RangeUtil::MergeRangeToVector(vec_WordDocRangeTimes,wordDocRangeTimes)!=-1);//一直往下合并
                            map_WordInDocRangeVector[docID] = vec_WordDocRangeTimes;
                        }
                    }
                    std::vector<PAIRDOCRANGETIMES> vec_WordDocRangeTimes = map_WordInDocRangeVector[docID];
                    //将一个词语的合并位置计数为1，并添加到最终要合并的集合中进行合并。
                    for(int n=0; n<vec_WordDocRangeTimes.size(); n++)
                    {
                        PAIRDOCRANGETIMES pair_DocRangeTimes = vec_WordDocRangeTimes[n];
                        pair_DocRangeTimes.second = 1;//计数重置为1

                        if(map_DocRangeVector.find(docID) == map_DocRangeVector.end())
                        {
                            std::vector<PAIRDOCRANGETIMES> vec_DocRangeTimes;
                            vec_DocRangeTimes.push_back(pair_DocRangeTimes);
                            map_DocRangeVector[docID] = vec_DocRangeTimes;
                        }
                        else
                        {
                            std::vector<PAIRDOCRANGETIMES> vec_DocRangeTimes = map_DocRangeVector[docID];//某个文档已保存的位置向量
                            while(RangeUtil::MergeRangeToVector(vec_DocRangeTimes,pair_DocRangeTimes)!=-1);
                            map_DocRangeVector[docID] = vec_DocRangeTimes;
                        }
                    }
                    map_DocWordNoPosition[docID] = map_NoPositionInDoc;
                }
            }
            //挑选出现次数大于阈值的短语范围
            LongestSimilarSentence* lss = new LongestSimilarSentence();
            std::map<DOC_ID,Document*> map_DocIDDocument;//文档ID和文档的内容映射，为了防止多次读取文档内容
            for(std::map<DOC_ID,std::vector<PAIRDOCRANGETIMES> >::iterator it = map_DocRangeVector.begin(); it != map_DocRangeVector.end(); it++)
            {
                DOC_ID docID = it->first;
                std::vector<PAIRDOCRANGETIMES> vec_DocRangeTimes = it->second;
                for(int m=0; m<vec_DocRangeTimes.size(); m++)
                {
                    PAIRDOCRANGETIMES docRangeTimes = vec_DocRangeTimes[m];
                    //if(docRangeTimes.second > 0.5*sen.vec_splitedHits.size())
                    if(docRangeTimes.second > KGRAM)//超过4个相同的就计算相似度
                    {
                        Range range_Sim = docRangeTimes.first;//相似句子的范围
                        std::map<int,Range> map_NoPositionInDoc = map_DocWordNoPosition[docID];
                        int n_SimDocBegin = map_NoPositionInDoc[range_Sim.begin].begin;
                        int n_SimDocEnd = map_NoPositionInDoc[range_Sim.end].end;
                        //std::cout<<str_Search<<std::endl;
                        //std::cout<<docID<<"\t["<<range_Sim.begin<<","<<range_Sim.end<<"]"<<std::endl;
                        //std::cout<<docID<<"\t["<<n_SimDocBegin<<","<<n_SimDocEnd<<"]"<<std::endl;
                        Document* docDB;
                        if(map_DocIDDocument.find(docID) == map_DocIDDocument.end())
                        {
                            docDB = new Document(docID);
                            map_DocIDDocument[docID] = docDB;
                        }
                        else
                        {
                            docDB = map_DocIDDocument[docID];
                        }
                        std::string str_Similar = docDB->GetstrContents().substr(n_SimDocBegin,n_SimDocEnd-n_SimDocBegin);
                        //std::cout<<str_Similar<<std::endl;
                        std::vector<PAIRSENRANGE> vec_PairSenRange;
                        double d_similarity = lss->GetSimBoundary(str_Search,str_Similar,vec_PairSenRange);
                        //取出相似句子保存
                        for(int x=0; x<vec_PairSenRange.size(); x++)
                        {
                            PAIRSENRANGE pair_SenRange = vec_PairSenRange[x];
                            //leakDoc
                            Range range_SeachSen = pair_SenRange.first;
                            int n_SearchSenBegin = sen.vec_splitedHits[range_SeachSen.begin].textRange.offset;//相似句子段在文章中的偏移值
                            int n_SearchSenEnd = sen.vec_splitedHits[range_SeachSen.end].textRange.offset + sen.vec_splitedHits[range_SeachSen.end].textRange.length;
                            TextRange textrange_SearchDoc = {n_SearchSenBegin, n_SearchSenEnd - n_SearchSenBegin};//范围
                            std::string str_Search = doc->GetstrContents().substr(n_SearchSenBegin,n_SearchSenEnd-n_SearchSenBegin);
                            // db doc
                            Range range_SimSen = pair_SenRange.second;
                            DOC_ID docID_DB = docID;
                            int n_SimWordNoBegin = range_Sim.begin + range_SimSen.begin;//相似句子段在文章中的编号
                            int n_SimWordNoEnd = range_Sim.begin + range_SimSen.end;
                            int n_SimSenBegin = map_NoPositionInDoc[n_SimWordNoBegin].begin;//相似句子段在文章中的偏移值
                            int n_SimSenEnd = map_NoPositionInDoc[n_SimWordNoEnd].end;
                            TextRange textrange_SimDoc = {n_SimSenBegin, n_SimSenEnd - n_SimSenBegin};//范围
                            std::string str_Similar = docDB->GetstrContents().substr(n_SimSenBegin,n_SimSenEnd - n_SimSenBegin);//原始句子

                            SimilarDocRange similarDocRange;//相似文档的范围
                            similarDocRange.str_Search = str_Search;
                            similarDocRange.textrange_SearchDoc = textrange_SearchDoc;
                            similarDocRange.docID_DB = docID_DB;
                            similarDocRange.str_Similar = str_Similar;
                            similarDocRange.textrange_SimilarDoc = textrange_SimDoc;
                            similarDocRange.similarity = d_similarity;
                            vec_SimilarDocRange.push_back(similarDocRange);
                        }
                    }
                }
            }
            //释放读取的文档资源信息
            for(std::map<DOC_ID,Document*>::iterator it = map_DocIDDocument.begin(); it != map_DocIDDocument.end(); it++)
            {
                delete it->second;
            }
            delete lss;
        }
    }
    return vec_SimilarDocRange;
}

DocumentDao::~DocumentDao()
{
    //dtor
}
