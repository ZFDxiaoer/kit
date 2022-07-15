#ifndef UTIL_DB_H
#define UTIL_DB_H

#include "SqlManager.h"
#include <vector>
#include <map>

vector<string> getNameById(uint32_t iUid)
{
    string query = "select username, nickname from t_user where uid = " + to_string(iUid);
    uint32_t iRet;
    mysqlpp::StoreQueryResult res;
    iRet = SQL_MANAGER->dbQuery(query, res);
    if(iRet !=0 || res.empty())
    {
        return {};
    }
    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    mysqlpp::Row row = *it;
    return {row[0].c_str(),row[1].c_str()};
}

vector<string> getUserNameByIdBatch(const vector<uint32_t>& vUid)
{
    if (vUid.empty())
    {
        return {};
    }
    string query = "select username from t_user where uid in (";
    bool first = true;
    for (auto uid : vUid)
    {
        if (first)
            first = false;
        else
            query += ",";
        query += to_string(uid);
    }
    query += ")";

    uint32_t iRet;
    mysqlpp::StoreQueryResult res;
    iRet = SQL_MANAGER->dbQuery(query, res);
    if(iRet !=0 || res.empty())
    {
        return {};
    }
    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    vector<string> vName;
    for (; it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        vName.push_back(row[0].c_str());
    }
    return vName;
}

map<uint32_t, vector<string> > getNameByIdBatch(const vector<uint32_t>& vUid)
{
    map<uint32_t, vector<string> > mResult;
    if (vUid.empty())
    {
        return mResult;
    }
    string query = "select uid, username, nickname from t_user where uid in (";
    bool first = true;
    for (auto uid : vUid)
    {
        if (first)
            first = false;
        else
            query += ",";
        query += to_string(uid);
    }
    query += ")";

    uint32_t iRet;
    mysqlpp::StoreQueryResult res;
    iRet = SQL_MANAGER->dbQuery(query, res);
    if(iRet !=0 || res.empty())
    {
        return mResult;
    }
    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    for (; it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        vector<string> &vName = mResult[row[0]];
        vName.push_back(row[1].c_str());
        vName.push_back(row[2].c_str());
    }
    return mResult;
}

bool changeNickName(uint32_t iUid, string& sNewNickName)
{
    string execute = "update t_user set nickname = '" + sNewNickName + "' where uid = " + to_string(iUid);
    if(SQL_MANAGER->dbUpdate(execute))
    {
        return true;
    }
    else
    {
        return false;
    }
   
}

#endif