#ifndef IDFDAO_H
#define IDFDAO_H

#include <fstream>
#include "mongo/client/dbclient.h"

#include "DataStructure.h"
#include "StringUtil.h"

class IDFDao
{
    public:
        IDFDao();
        virtual ~IDFDao();

        int ReadIDFToDB(const std::string str_path);
        int Insert(const std::string str_word, const double d_idf);
        double GetIDF(const std::string str);
    protected:
    private:
        mongo::DBClientConnection m_Conn;
        std::string m_Host;
        std::string m_DBName;
};

#endif // IDFDAO_H
