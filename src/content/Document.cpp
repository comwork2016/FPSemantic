#include "Document.h"

/**
    只读取文件中的一段内容
*/
Document::Document(const std::string& str_DocPath,TextRange range)
{
    this->m_DocID = str_DocPath;
    this->m_strDocPath = str_DocPath;
    int n_SeparatorIndex = str_DocPath.find_last_of("/");
    this->m_strDocName = str_DocPath.substr(n_SeparatorIndex+1);
    this->m_strContents = "";
    this->m_lSimHash = 0;
    this->m_nWordCount = 0;
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        std::cout<<"read file "<<this->m_strDocPath<<" error"<<std::endl;
        exit(ERROR_OPENFILE);
    }
    char* buf = new char[range.length+1];
    ifs_Doc.seekg(range.offset,std::ios_base::beg);
    ifs_Doc.read(buf,range.length);
    buf[range.length] = '\0';
    this->m_strContents = buf;
    delete[] buf;
}

Document::Document(const std::string& str_DocPath,bool b_SplitToSentence)
{
    //ctor
    this->m_DocID = str_DocPath;
    this->m_strDocPath = str_DocPath;
    int n_SeparatorIndex = str_DocPath.find_last_of("/");
    this->m_strDocName = str_DocPath.substr(n_SeparatorIndex+1);
    this->m_strContents = "";
    this->m_lSimHash = 0;
    this->m_nWordCount = 0;
    std::cout<<"Reading document "<<this->m_strDocName<<std::endl;
    if(!b_SplitToSentence)
    {
        int n_ReadStats = ReadDocumentContent();
        if(n_ReadStats == ERROR_OPENFILE)
        {
            std::cout<<"read file "<<this->m_strDocPath<<" error"<<std::endl;
            exit(ERROR_OPENFILE);
        }
    }
    else
    {
        //读取文档内容
        int n_ReadStats = ReadDocumentAndSplitToSentence();
        if(n_ReadStats == ERROR_OPENFILE)
        {
            std::cout<<"read file "<<this->m_strDocPath<<" error"<<std::endl;
            exit(ERROR_OPENFILE);
        }
    }
}

/**
    一次性读取全部的文档内容
*/
int Document::ReadDocumentContent()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    std::stringstream ss;
    ss<<ifs_Doc.rdbuf();
    this->m_strContents = ss.str();
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    读取文件内容
    程序中，将一行内容作为一个段落
    并用常见符号将段落分割成句子
*/
int Document::ReadDocumentAndSplitToSentence()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    int offset=0;
    while(!ifs_Doc.eof())
    {
        Paragraph para;
        para.textRange.offset = offset;
        //读入一行数据
        std::string str_Line;
        std::getline(ifs_Doc,str_Line);
        int n_LineLength = str_Line.length();
        para.textRange.length = n_LineLength;
        this->m_strContents.append(str_Line);
        offset+=n_LineLength;
        // 不是文章结尾时将原文档的换行符加回去
        if(!ifs_Doc.eof())
        {
            this->m_strContents.append(1,'\n');
            offset++;
        }
        if(n_LineLength != 0 && !StringUtil::isStringBlank(str_Line)) //空白行不计为段落
        {
            //对段落进行句子的拆分
            SplitParaphToSentence(para);
            this->m_vecParagraph.push_back(para);
        }
    }
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    将段落分割成句子
*/
void Document::SplitParaphToSentence(Paragraph& para)
{
    std::string str = this->m_strContents.substr(para.textRange.offset,para.textRange.length);
    //将文档用标点符号拆分
    std::vector<std::string> vec_pattern;
    vec_pattern.push_back("。");
    vec_pattern.push_back("？");
    vec_pattern.push_back("！");
    vec_pattern.push_back("；");
    const int strsize=str.size();
    std::string::size_type pos;
    int i = 0;
    while(i<strsize)
    {
        pos = strsize;
        int patternSize=0;
        //查找第一个分隔符的位置
        for(int j=0; j<vec_pattern.size(); j++)
        {
            std::string pattern = vec_pattern[j];
            int index =str.find(pattern,i);
            if(index!=std::string::npos && index < pos)
            {
                pos = index;
                patternSize = pattern.size();
            }
        }
        if(pos<=strsize)
        {
            std::string s=str.substr(i,pos-i);
            if(!StringUtil::isStringBlank(s))
            {
                Sentence sen;
                sen.textRange.offset = para.textRange.offset + i;
                sen.textRange.length = pos-i;
                sen.firstWordNo = 0;
                para.vec_Sentences.push_back(sen);
            }
            i=pos+patternSize;
        }
    }
}

/**
    计算全文的simhash
*/
void Document::CalcDocSimHash()
{
    std::cout<<"Calculate simhash of document "<<this->m_strDocName<<std::endl;
    std::vector<SIMHASH_TYPE> vec_KGramHash;
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = this->m_vecParagraph[i];
        //将句子组合成一个特征项，并添加到文档指纹集合中
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            int n_WordCount = sen.vec_splitedHits.size();
            for(int x=0; x<n_WordCount-SIMHASHKGRAM+1; x++)
            {
                std::string kgramWord;
                for(int k=0; k<SIMHASHKGRAM; k++)
                {
                    SplitedHits sh = sen.vec_splitedHits[x+k];
                    kgramWord.append(sh.word);
                }
                SIMHASH_TYPE simhash = HashUtil::CalcStringHash(kgramWord);
                vec_KGramHash.push_back(simhash);
            }
        }
    }
    this->m_lSimHash = HashUtil::CalcDocSimHash(vec_KGramHash);
    //将simhash分成4部分，分别保存起来，便于海量数据的查询
    this->m_lSimHash16_1  = this->m_lSimHash & 0xFFFF000000000000;
    this->m_lSimHash16_2  = this->m_lSimHash & 0xFFFF00000000;
    this->m_lSimHash16_3  = this->m_lSimHash & 0xFFFF0000;
    this->m_lSimHash16_4  = this->m_lSimHash & 0xFFFF;
}

/**
    计算词语的逆文档频率
*//*
void Document::CalcTFIDF()
{
    for(std::map<std::string,double>::iterator it = this->m_mapTFIDF.begin(); it != this->m_mapTFIDF.end(); it++)
    {
        std::string str_term = it->first;
        double d_TF = it->second / this->m_nWordCount;
        this->m_mapTFIDF[str_term] = d_TF;
    }
}*/

/**
    将句子分成词语块，便于小粒度查找泄露文档
*//*
void Document::SplitSentencesToKGrams()
{
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = this->m_vecParagraph[i];
        for(int j=0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            if(sen.vec_splitedHits.size()<KGRAM)//如果分词数少于阈值，则不作为一个特征项
            {
                return;
            }
            //遍历分词列表
            for(int m=0; m < sen.vec_splitedHits.size() - KGRAM + 1; m++)
            {
                KGram kgram;
                for(int n=m; n < m + KGRAM; n++)
                {
                    kgram.vec_splitedHits.push_back(sen.vec_splitedHits[n]);
                }
                kgram.textRange.offset = kgram.vec_splitedHits[0].textRange.offset;
                kgram.textRange.length = kgram.vec_splitedHits[KGRAM-1].textRange.offset +kgram.vec_splitedHits[KGRAM-1].textRange.length;
                sen.vec_KGram.push_back(kgram);
            }
        }
    }
}*/

/**
    构造倒排索引
*/
void Document::BuildInvertedIndex()
{
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph para = this->m_vecParagraph[i];
        for(int j=0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            for(int k=0; k<sen.vec_splitedHits.size(); k++)
            {
                SplitedHits sh = sen.vec_splitedHits[k];
                WordIndex* wordsIndex;
                if(this->m_mapWordIndex.find(sh.word)==this->m_mapWordIndex.end())//文档中第一次出现该单词
                {
                    wordsIndex = new WordIndex(sh.word,sh.textRange.length,sh.POS);
                }
                else
                {
                    wordsIndex = this->m_mapWordIndex[sh.word];
                }
                WordPos wordPos = {sh.textRange.offset,sh.NoInDoc,sen.textRange.offset};
                wordsIndex->AddDocPosInfo(this->m_strDocPath,wordPos);
                this->m_mapWordIndex[sh.word] = wordsIndex;
            }
        }
    }
}

/**
    输出文件的信息
*/
void Document::Display() const
{
    std::cout<<this->m_strDocName<<std::endl;
    //输出段落句子的信息
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph para = m_vecParagraph[i];
        //对句子进行分词并计算simhash
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            std::string str_sentence = this->m_strContents.substr(sen.textRange.offset,sen.textRange.length);
            std::cout<<"Para "<<i<<" Sentence "<<j<<":["<<sen.textRange.offset<<","<<sen.textRange.length<<"]"<<std::endl;
            std::cout<<str_sentence<<std::endl<<std::endl;
            for(int k=0; k<sen.vec_splitedHits.size(); k++)
            {
                SplitedHits sh = sen.vec_splitedHits[k];
                std::cout<<sh.NoInDoc<<": "<<sh.word<<"["<<sh.textRange.offset<<","<<sh.textRange.length<<","<<sh.POS<<"]"<<"\t";
                //std::cout<<sh.word<<"/"<<sh.pos<<"\t";
            }
            std::cout<<std::endl<<std::endl;
            /*for(int k=0; k<sen.vec_KGram.size(); k++)
            {
                KGram kgram = sen.vec_KGram[k];
                std::cout<<kgram.textRange.offset<<","<<kgram.textRange.offset + kgram.textRange.length<<"\t"<<std::endl;
                for(int x=0; x < KGRAM; x++)
                {
                    SplitedHits sh = kgram.vec_splitedHits[x];
                    std::cout<<sh.word<<"\t";
                }
                std::cout<<std::endl;
            }*/
        }
    }
    /*查看倒排索引信息*/
    std::map<std::string,WordIndex*> map_WordIndex = this->m_mapWordIndex;
    for(std::map<std::string,WordIndex*>::iterator it = map_WordIndex.begin(); it != map_WordIndex.end(); it++)
    {
        WordIndex* wordIndex = it->second;
        wordIndex->Display();
        std::cout<<std::endl;
    }
    std::cout<<this->m_lSimHash<<std::endl;
}

Document::~Document()
{
    //释放倒排索引所占的资源
    for(std::map<std::string,WordIndex*>::iterator it = this->m_mapWordIndex.begin(); it != this->m_mapWordIndex.end(); it++)
    {
        delete it->second;
    }
}
