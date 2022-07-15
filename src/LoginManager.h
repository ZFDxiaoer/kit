#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include "util_singleton.h"
#include <string>
#include <map>
#include "time.h"

#include "cmd.pb.h"

using namespace std;

class LoginManager : public CSingleton<LoginManager>
{
    public:
        uint32_t login(const string& username, const string& passwd, uint32_t& iUid);
        uint32_t registry(const string& sClientIP, const string& username, const string& nickname, const string& passwd);

    private:
        uint32_t checkFreeze(const string& username);
        uint32_t onPasswError(const string& username);
        uint32_t checkAlreadyLogin(const string& username);

        uint32_t checkRegistry(const string& sClientIP);
        uint32_t onRegistry(const string& sClientIP);
    
    private:
        map<string, pair<time_t, uint32_t> > m_mPasswLoginNums; // username, time, num
        map<string, pair<time_t, uint32_t> > m_mRegistryNums;   // ip, time, num
};

#define LOGIN_MANAGER LoginManager::GetInstance()

#endif