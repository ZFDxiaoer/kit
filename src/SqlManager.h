#ifndef SQL_MANAGER_H
#define SQL_MANAGER_H

#include <mysql++.h>
#include <iostream>
#include <string>
#include "cmd.pb.h"

#include "util_singleton.h"
// #include "../src/LogManager.h"



class SqlManager:public CSingleton<SqlManager> {

private:
    std::string           m_dbName     = "michatdb";
    std::string           m_dbUser     = "zhaofudi";
    std::string           m_dbPassword = "123456";
    std::string           m_dbAddress  = "localhost";
    int                   m_dbPort     = 3306;
    bool                  m_flag       = false;
    mysqlpp::Connection   m_conn       = false;
    
public:
    SqlManager();
    void dbConnectServer();
    bool dbPing();
    int32_t dbQuery(std::string sqlString, mysqlpp::StoreQueryResult& res);
    int32_t dbInsert(std::string sqlStirng);
    int32_t dbUpdate(std::string sqlString);
    int32_t dbDelete(std::string sqlString);
    int32_t dbExecute(std::string sqlString);
};


#define SQL_MANAGER SqlManager::GetInstance()

#endif //SQL_MANAGER_H
