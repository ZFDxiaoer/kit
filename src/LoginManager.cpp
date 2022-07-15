#include "LoginManager.h"
#include "SqlManager.h"
#include "LogManager.h"
#include "SessionManager.h"

uint32_t LoginManager::login(const string& username, const string& passwd, uint32_t& iUid)
{
    iUid = 0;
    uint32_t iRet = checkFreeze(username);
    if (iRet != 0)
    {
        return iRet;
    }

    mysqlpp::StoreQueryResult res;

    string query = "select uid, username, passwd from t_user where username='" + username + "'";

    iRet = SQL_MANAGER->dbQuery(query, res);
    if (iRet != 0)
    {
        return pb::LoginRsp::RET_FAIL;
    }

    if (res.empty())
    {
        return pb::LoginRsp::RET_ACCOUNT_ERROR;
    }

    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    mysqlpp::Row row = *it;

    // 检查是否已经登陆
    if(SESSION_MANAGER->checkAlreadyLogin(row[0]))
    {
        return pb::LoginRsp::RET_ACCOUNT_ONLINE;
    }
    if (row[2] != passwd)
    {
        iRet = onPasswError(username);
        if (iRet != 0)
        {
            return iRet;
        }
        return pb::LoginRsp::RET_ACCOUNT_ERROR;
    }
    
    iUid = row[0];
    return 0;
}

uint32_t LoginManager::registry(const string& sClientIP, const string& username, const string& nickname, const string& passwd)
{
    uint32_t iRet = checkRegistry(sClientIP);
    if (iRet != 0)
    {
        return iRet;
    }

    if (username.length() >= 20 || username.length() <= 3)
    {
        return pb::RegisterRsp::RET_USERNAME_LEN_ERROR;
    }

    if(nickname.length() >= 20 || nickname.length() <=3 )
    {
        return pb::RegisterRsp::RET_NICKNAME_LEN_ERROR;
    } 

    if (passwd.length() < 6)
    {
        return pb::RegisterRsp::RET_PASSWD_TOO_SHORT;
    }

    mysqlpp::StoreQueryResult res;

    string query = "select uid from t_user where username='" + username + "'";

    iRet = SQL_MANAGER->dbQuery(query, res);
    if (iRet != 0)
    {
        return pb::RegisterRsp::RET_FAIL;
    }

    if (!res.empty())
    {
        return pb::RegisterRsp::RET_USERNAME_REPEAT;
    }

    string execute = "insert into t_user (username, nickname, passwd, register_ip) values ('" + 
    username + "','" + nickname + "','" + passwd + "','" + sClientIP + "')";
    iRet = SQL_MANAGER->dbExecute(execute);
    if (iRet != 0)
    {
        return pb::RegisterRsp::RET_FAIL;
    }

    onRegistry(sClientIP);
    return 0;
}

uint32_t LoginManager::checkFreeze(const string& username)
{
    time_t now = time(NULL);
    auto iter = m_mPasswLoginNums.find(username);
    if (iter != m_mPasswLoginNums.end())
    {
        time_t iLastErrorTime = iter->second.first;
        if (iLastErrorTime < now && now - iLastErrorTime < 30 && iter->second.second >= 5)
        {
            // 30秒内登录错误5次，就封了
            GLOBALINFO("login too offen, freeze");
            return pb::LoginRsp::RET_ACCOUNT_FREEZE;
        }
    }
    return 0;
}


uint32_t LoginManager::checkAlreadyLogin(const string& username)
{
    return 0;
}

uint32_t LoginManager::onPasswError(const string& username)
{
    time_t now = time(NULL);
    auto iter = m_mPasswLoginNums.find(username);
    if (iter == m_mPasswLoginNums.end())
    {
        m_mPasswLoginNums[username] = make_pair(now, 1);
        return 0;
    }
    time_t iLastErrorTime = iter->second.first;
    if (iLastErrorTime > now || now - iLastErrorTime > 30)
    {
        iter->second.first = now;
        iter->second.second = 1;
        return 0;
    }
    iter->second.second += 1;
    if (iter->second.second >= 5)
    {
        GLOBALINFO("login too offen, freeze");
        return pb::LoginRsp::RET_ACCOUNT_FREEZE;
    }
    return 0;
}

uint32_t LoginManager::checkRegistry(const string& sClientIP)
{
    time_t now = time(NULL);
    auto iter = m_mRegistryNums.find(sClientIP);
    if (iter != m_mRegistryNums.end())
    {
        time_t iLastErrorTime = iter->second.first;
        if (iLastErrorTime < now && now - iLastErrorTime < 30 && iter->second.second >= 5)
        {
            // 30秒内注册过5次，就封了
            return pb::RegisterRsp::RET_REGISTER_TOO_OFFEN;
        }
    }
    return 0;
}

uint32_t LoginManager::onRegistry(const string& sClientIP)
{
    time_t now = time(NULL);
    auto iter = m_mRegistryNums.find(sClientIP);
    if (iter == m_mRegistryNums.end())
    {
        m_mRegistryNums[sClientIP] = make_pair(now, 1);
        return 0;
    }
    time_t iLastErrorTime = iter->second.first;
    if (iLastErrorTime > now || now - iLastErrorTime > 30)
    {
        iter->second.first = now;
        iter->second.second = 1;
        return 0;
    }
    iter->second.second += 1;
    if (iter->second.second >= 5)
    {
        return pb::RegisterRsp::RET_REGISTER_TOO_OFFEN;
    }
    return 0;
}
