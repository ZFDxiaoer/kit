#include "SessionManager.h"
#include "LogManager.h"
#include "ChatManager.h"

void SessionManager::onConnect(uint32_t iUid, Session_ptr pSession)
{
    if (iUid == 0)
    {
        return;
    }
    GLOBALINFO("[onConnect]-iUid: " << iUid);
    auto iter = mSessions.find(iUid);
    if (iter != mSessions.end())
    {
        // 强制下线
        iter->second->disConnect();
        // 后面iter不可再用
    }
    mSessions[iUid] = pSession;

    // 开始清理Chat部分
    CHAT_MANAGER->deleteMapByUserId(iUid);
}

void SessionManager::onDisConnect(uint32_t iUid)
{
    if (iUid == 0)
    {
        return;
    }

    mSessions.erase(iUid);
    
    // 开始清理Chat部分
    CHAT_MANAGER->deleteMapByUserId(iUid);
}

void SessionManager::sendByUid(uint32_t iUid, const pb::CmdData& stCmdData)
{
    GLOBALINFO("[SessionManager::sendByUid], iUid: " << iUid << " msg: " << stCmdData.SerializeAsString());
    auto iterSessionPtr = mSessions.find(iUid);
    if (iterSessionPtr != mSessions.end())
    {
        iterSessionPtr->second->send(stCmdData);
    }
}


bool SessionManager::checkAlreadyLogin(uint32_t iUid)
{
    if(mSessions.find(iUid) == mSessions.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}