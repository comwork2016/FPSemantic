#include "IDFDao.h"

/**
    工程中没有用到
*/

IDFDao::IDFDao()
{
    //ctor
    this->m_Host = "localhost:27017";
    this->m_DBName = "FP.idf";
    this->m_Conn.connect(this->m_Host);
}

/**
    将glossary.dat文件中的概念信息读入到数据库中
*/
int IDFDao::ReadIDFToDB(const std::string str_path)
{
    std::cout<<"Reading idf corpus!"<<std::endl;
    std::ifstream fin;
    fin.open(str_path.c_str(),std::ios::in);
    if(!fin.is_open())
    {
        std::cout<<"Read file: "<<str_path<<" error!"<<std::endl;
        exit(ERROR_OPENFILE);
    }
    std::string line;
    while (getline(fin, line))
    {
        std::vector<std::string> fields = StringUtil::SplitString(line,",");
        std::string str_word = StringUtil::Trim(fields[0]);
        std::string str_idf = StringUtil::Trim(fields[1]);
        double d_idf = atof(str_idf.c_str());
        Insert(str_word,d_idf);
    }
}

/**
    插入一行glossary数据到数据库中
*/
int IDFDao::Insert(const std::string str_word, const double d_idf)
{
    mongo::BSONObjBuilder b;
    //保存文档信息
    b.append("word",str_word);
    b.appendNumber("idf",d_idf);
    this->m_Conn.insert(this->m_DBName,b.obj());
    return 0;
}

/**
    查询词语的逆文档频率
*/
double IDFDao::GetIDF(const std::string str)
{
    double d_idf;
    mongo::Query query = QUERY("word"<<str);
    mongo::BSONObj bo_columns = BSON("idf"<<1);
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->m_Conn.query(this->m_DBName,query,0,0,&bo_columns);
    while(cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        d_idf = p.getField("idf").numberDouble();
    }
    return d_idf;
}

IDFDao::~IDFDao()
{
    //dtor
}
