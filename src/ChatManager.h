#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include <string>
#include <vector>
#include <deque>
#include <algorithm>

#include "util_singleton.h"
#include "cmd.pb.h"


#define MAX_RECENT_MESSAGES 10

using namespace std;

class ChatManager : public CSingleton<ChatManager>
{
    public:
        ChatManager(){
            //createRoom();
        }


        uint32_t createRoom(uint32_t iUid, uint32_t iRoomId);
        uint32_t deleteRoom(uint32_t iUid, uint32_t iRoomId);
        uint32_t joinRoom(uint32_t iUid, uint32_t iRoomId, vector<string>& vUserName, vector<pb::ChatMsg>& vMsgList);
        uint32_t leaveRoom(uint32_t iUid);
        uint32_t changeRoom(uint32_t iUid, uint32_t iDestRoomId);
        vector<uint32_t> getRoomList();
        vector<pb::ChatMsg> getRoomMsgListByRoomId(uint32_t iRoomId);
        uint32_t sendChat(uint32_t iUid, const pb::SendMsgReq& stReq);
        vector<uint32_t> getRoomUser(uint32_t iRoomId);
        uint32_t changeNickName(uint32_t iUid, string sNewName);
        void deleteMapByUserId(uint32_t iUid);   //如果session断了，要清理m_mRoom2Uid,m_mUid2Room两个数据结构

    private:
        uint32_t getUserRoomId(uint32_t iUid);
        void sendToRoomAll(uint32_t iUid, uint32_t iRoomId, const pb::CmdData& stCmdData);
        bool checkRoomExist(uint32_t iRoomId);

    private:
        vector<uint32_t> m_vRoom;
        map<uint32_t, vector<uint32_t>> m_mRoom2Uid; // room id -> room user list
        map<uint32_t, uint32_t> m_mUid2Room;
};

#define CHAT_MANAGER ChatManager::GetInstance()

#endif