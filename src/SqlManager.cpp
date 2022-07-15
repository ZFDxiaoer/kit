#include "SqlManager.h"
#include "LogManager.h"

SqlManager::SqlManager()
{
    dbConnectServer();
}


void SqlManager::dbConnectServer()
{
    int number = 0;
    m_flag = false;

    // 最多尝试连接10次
    do
    {
        if(m_conn.connect(m_dbName.c_str(), m_dbAddress.c_str(), m_dbUser.c_str(), m_dbPassword.c_str()))
        {
            m_flag = true;

            GLOBALDEBUG("db connect suceess");

            break;
        }
        else
        {
            GLOBALERROR("db connect error, number: " << number);
        }
        ++number;
    
    } while (!m_flag && number < 10);
    
}

bool SqlManager::dbPing()
{
    if(!m_conn.ping()) 
    {
        dbConnectServer();
        if(!m_flag) 
        {
            return false; 
        }
    }
    return true;
}

int32_t SqlManager::dbQuery(std::string sqlString, mysqlpp::StoreQueryResult& res)
{
    
    if(!dbPing())
    {
        // ToDo errer
        return pb::ErrorCode::ErrorCode_Net_DbError; // 数据库连接错误码
    }

    // query 查询
    mysqlpp::Query query = m_conn.query(sqlString);
    if(res = query.store())
    {
        GLOBALINFO("数据库查询结果正常, " << sqlString);

        // ToDo
        return 0; // 查询成功
    }
    else
    {
        GLOBALERROR( "数据库查询错误, sql:" << sqlString << " errorNum: " << m_conn.errnum() << " error: " <<
        m_conn.error());
       
        // ToDo errer
        return pb::ErrorCode::ErrorCode_Net_DbError; // 数据库连接错误码
    }
    
}


int32_t SqlManager::dbInsert(std::string sqlString)
{
    GLOBALDEBUG("数据库插入, sql: " << sqlString);

    return dbExecute(sqlString);
}

int32_t SqlManager::dbUpdate(std::string sqlString)
{
    return dbExecute(sqlString);
}

int32_t SqlManager::dbDelete(std::string sqlString)
{
    return dbExecute(sqlString);
}

int32_t SqlManager::dbExecute(std::string sqlString)
{
    if(!dbPing())
    {
        // ToDo errer
        return pb::ErrorCode::ErrorCode_Net_DbError; // 数据库连接错误码
    }

    // execute 执行
    mysqlpp::Query query = m_conn.query(sqlString);
    if(query.exec())
    {
        GLOBALDEBUG("数据库执行结果正常, " << sqlString);
        // ToDo
        return 0; // 执行成功
    }
    else
    {
        GLOBALERROR("数据库执行结果错误, sql:" << sqlString);
        // ToDo errer
        return pb::ErrorCode::ErrorCode_Net_DbError; // 执行错误码
    }
}
