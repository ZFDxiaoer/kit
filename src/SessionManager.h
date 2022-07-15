#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "Session.h"
#include <string>
#include <map>
#include <list>
#include "util_singleton.h"

#include "cmd.pb.h"

using namespace std;

typedef std::shared_ptr<Session> Session_ptr;

struct SessionWriteQuest
{
    SessionWriteQuest(uint32_t uid, const pb::CmdData& cmd) :
    iUid(uid), stCmd(cmd)
    {}

    uint32_t iUid;
    pb::CmdData stCmd;
};

class SessionManager : public CSingleton<SessionManager>
{
    public:
        void onConnect(uint32_t, Session_ptr);
        void onDisConnect(uint32_t);
        void sendByUid(uint32_t, const pb::CmdData&);
        bool checkAlreadyLogin(uint32_t);
    private:
        map<uint32_t, Session_ptr> mSessions;
        
};

#define SESSION_MANAGER SessionManager::GetInstance()

#endif