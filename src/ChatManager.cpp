#include "ChatManager.h"
#include "SessionManager.h"
#include "LogManager.h"
#include "SqlManager.h"
#include "util_db.h"
uint32_t Session::iUidCnt = 1;


uint32_t ChatManager::createRoom(uint32_t iUid, uint32_t iRoomId)
{
    GLOBALDEBUG("ChatManager::createRoom, iUid:" << iUid << " iRoomId:" << iRoomId);
    string execute = "insert into t_room (rid, creator_id) values ('" +
            to_string(iRoomId) + "','" + to_string(iUid) +"')";
    uint32_t iRet = SQL_MANAGER->dbInsert(execute);

    if(iRet != 0) 
    {
        //ToDo error ,创建房间错误
        return 1;
    }

    string query = "select rid from t_room where creator_id = '" + to_string(iUid) + "' order by create_time desc limit 1";
    mysqlpp::StoreQueryResult res;
    iRet = SQL_MANAGER->dbQuery(query, res);

    if(iRet != 0 || res.empty()) 
    {
        //ToDo error ,创建房间错误
        return 1;
    }

    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    mysqlpp::Row row = *it;
    iRoomId = row[0];
    m_vRoom.push_back(iRoomId);
    m_mRoom2Uid[iRoomId] = {};
    return 0;
}

uint32_t ChatManager::deleteRoom(uint32_t iUid, uint32_t iRoomId)
{

    // 目前不允许删除房间，删除房间逻辑还比较复杂
    return 0;

    string query = "select * from t_room where creator_id = '" + to_string(iUid) + 
    "' and rid = " + to_string(iRoomId)+ "'";
    string execute = "update ";

    auto itVec = find(m_vRoom.begin(), m_vRoom.end(), iRoomId);
    if(itVec == m_vRoom.end())
    {
        //未找到
        return 0;
    }
    m_vRoom.erase(itVec);

    // 
    auto itMap = m_mRoom2Uid.find(iRoomId);
    if(itMap == m_mRoom2Uid.end())
    {
        // 未找到
        return 0;
    }
    m_mRoom2Uid.erase(itMap);

    for(auto iUid: m_mRoom2Uid[iRoomId])
    {
        leaveRoom(iUid);
    }

    return 1;
    
}


uint32_t ChatManager::joinRoom(uint32_t iUid, uint32_t iRoomId, vector<string>& vUserName, vector<pb::ChatMsg>& vMsgList)
{
    // 已经在其他的房间不可以加入
    if(getUserRoomId(iUid) != 0)
    {
        // ToDo error
        GLOBALERROR("[ChatManager::joinRoom], user: " << iUid <<  " also in room: " << iRoomId);
        return pb::EnterRoomRsp_Retcode_RET_FAIL;
    }

    // 检查房间是否存在
    if(!checkRoomExist(iRoomId))
    {
        // ToDo error
        GLOBALERROR("[ChatManager::joinRoom], room not exist: " << iRoomId << "begin create");
        uint32_t iRet = createRoom(iUid, iRoomId);
        if(iRet != 0)
        {
            return pb::EnterRoomRsp_Retcode_RET_FAIL;
        }
        else
        {
            return joinRoom(iUid, iRoomId, vUserName, vMsgList);
        }
    }

    m_mUid2Room[iUid] = iRoomId;
    m_mRoom2Uid[iRoomId].push_back(iUid);

    // 填充回复字段
    vector<uint32_t> vUserId = getRoomUser(iRoomId);
    vUserName = getUserNameByIdBatch(vUserId);
    vMsgList = getRoomMsgListByRoomId(iRoomId);

    // notify 其他人
    // 构造消息
    auto names = getNameById(iUid);
    pb::UserEnterNotify rsp;
    rsp.set_username(names[0]);
    rsp.set_nickname(names[1]);

    // UserEnterNotify -> stCmdData
    pb::CmdData stRsp;
    stRsp.set_cmdid(pb::CMD_USER_ENTER_NOTIFY);
    stRsp.set_cmddata(rsp.SerializeAsString());
    stRsp.set_ret(pb::CMD_USER_ENTER_NOTIFY);

    GLOBALDEBUG("[ChatManager::joinRoom], begin notify"); 
    sendToRoomAll(iUid, iRoomId, stRsp);

    return pb::EnterRoomRsp_Retcode_RET_SUCC;
}

uint32_t ChatManager::leaveRoom(uint32_t iUid)
{
    // m_mUid2Room 变更
    auto itMap = m_mUid2Room.find(iUid);
    if(itMap == m_mUid2Room.end())
    {
        // 用户未加入房间
        // ToDo error
        GLOBALERROR("[ChatManager::leaveRoom], user not in room, can't leave");
        return pb::LeaveRoomRsp_Retcode_RET_FAIL;
    }

    uint32_t iRoomId = itMap->second;

    // m_mRoom2Uid 变更
    auto itMap2 = m_mRoom2Uid.find(iRoomId);
    if(itMap2 == m_mRoom2Uid.end())
    {
        // 未找到房间
        // ToDo error
        GLOBALERROR("[ChatManager::leaveRoom], user not in room, can't leave");
        return pb::LeaveRoomRsp_Retcode_RET_FAIL;
    }

    auto itVec = find(itMap2->second.begin(), itMap2->second.end(), iUid);
    if(itVec == itMap2->second.end())
    {
        //未找到
        GLOBALERROR("[ChatManager::leaveRoom], user not in room, can't leave");
        // ToDo error
        return pb::LeaveRoomRsp_Retcode_RET_FAIL;
    }

    // notify 其他人
    // 构造消息
    auto names = getNameById(iUid);
    pb::UserLeaveNotify rsp;
    rsp.set_username(names[0]);
    rsp.set_nickname(names[1]);

    // UserLeaveNotify -> stCmdData
    pb::CmdData stRsp;
    stRsp.set_cmdid(pb::CMD_USER_LEAVE_NOTIFY);
    stRsp.set_cmddata(rsp.SerializeAsString());
    stRsp.set_ret(pb::CMD_USER_LEAVE_NOTIFY);

    // 更改相应数据结构
    m_mUid2Room.erase(iUid);
    itMap2->second.erase(itVec);

    GLOBALDEBUG("[ChatManager::leaveRoom], begin notify");
    // 发给房间内所有人，包括自己
    sendToRoomAll(iUid, iRoomId, stRsp);

    return pb::LeaveRoomRsp_Retcode_RET_SUCC;
}


uint32_t ChatManager::changeRoom(uint32_t iUid, uint32_t iDestRoomId)
{
    // 离开房间失败
    if(leaveRoom(iUid) == 0)
    {
        // ToDo error
        return 1;
    }

    vector<string> vUserName;
    vector<pb::ChatMsg> vMsgList;
    joinRoom(iUid, iDestRoomId, vUserName, vMsgList);
    return 0;
}


vector<uint32_t> ChatManager::getRoomList()
{
    return m_vRoom;
}


vector<pb::ChatMsg> ChatManager::getRoomMsgListByRoomId(uint32_t iRoomId)
{
    vector<pb::ChatMsg> vResult;
    string query = "select uid, msg from t_message where rid=" + to_string(iRoomId) + " order by mid desc limit 10";
    
    uint32_t iRet;
    mysqlpp::StoreQueryResult res;
    iRet = SQL_MANAGER->dbQuery(query, res);
    if(iRet !=0 || res.empty())
    {
        return vResult;
    }

    vector<uint32_t> vUid;
    vector<pair<uint32_t, pb::ChatMsg> > vTmpMsgList;
    mysqlpp::StoreQueryResult::const_iterator it = res.begin();
    for (; it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        pb::ChatMsg newMsg;
        newMsg.set_content(row[1]);
        vTmpMsgList.push_back(make_pair(row[0], newMsg));
        vUid.push_back(row[0]);
    }

    map<uint32_t, vector<string> > mUserInfo = getNameByIdBatch(vUid);
    for (auto tmpMsg : vTmpMsgList)
    {
        auto iter = mUserInfo.find(tmpMsg.first);
        if (iter != mUserInfo.end())
        {
            tmpMsg.second.set_username(iter->second.at(0));
            tmpMsg.second.set_nickname(iter->second.at(1));
            vResult.insert(vResult.begin(), tmpMsg.second);
        }
    }

    return vResult;
}

uint32_t ChatManager::sendChat(uint32_t iUid, const pb::SendMsgReq& stReq)
{
    GLOBALINFO("[ChatManager::sendChat], iUid: " << iUid << " msg: " << stReq.SerializeAsString());
    uint32_t iRoomId = getUserRoomId(iUid);
    if (iRoomId == 0)
    {
        // ToDo error
        return pb::SendMsgRsp_Retcode_RET_NOT_IN_ROOM; 
    }

    auto names = getNameById(iUid);
    if(names.empty()) 
    {
        return pb::SendMsgRsp_Retcode_RET_NOT_IN_ROOM; 
    } 

    // 存数据库
    string execute = "insert into t_message (rid, uid, msg) values (" +
            to_string(iRoomId) + "," + to_string(iUid) +",'" + stReq.msg() + "')";
    uint32_t iRet = SQL_MANAGER->dbInsert(execute);
    if(iRet != 0) 
    {
        //ToDo error 数据库错误
        return 1;
    }

    // 构造消息
    pb::MsgNotify nrsp;
    pb::ChatMsg* rsp = new pb::ChatMsg();
    rsp->set_username(names[0]);
    rsp->set_nickname(names[1]);
    time_t now = time(NULL);
    rsp->set_time(now);
    rsp->set_content(stReq.msg());

    // ChatMsg -> MsgNotify
    nrsp.set_allocated_msg(rsp);

    // MsgNotify -> stCmdData
    pb::CmdData stRsp;
    stRsp.set_cmdid(pb::CMD_SEND_MSG_REQ);
    stRsp.set_cmddata(nrsp.SerializeAsString());
    stRsp.set_ret(pb::CMD_MSG_NOTIFY);

    // 发给房间内所有人，包括自己
    sendToRoomAll(iUid, iRoomId, stRsp);

    GLOBALDEBUG("[ChatManager::sendChat], begin notify");
    return pb::SendMsgRsp_Retcode_RET_SUCC; 
}

uint32_t ChatManager::getUserRoomId(uint32_t iUid)
{
    auto itMap = m_mUid2Room.find(iUid);
    if(itMap == m_mUid2Room.end())
    {
        return 0;
    }
    return itMap->second;
}

vector<uint32_t> ChatManager::getRoomUser(uint32_t iRoomId)
{
    auto itMap = m_mRoom2Uid.find(iRoomId);
    if(itMap == m_mRoom2Uid.end())
    {
        return {};
    }
    return itMap->second;
}

void ChatManager::sendToRoomAll(uint32_t iUid, uint32_t iRoomId, const pb::CmdData& stCmdData)
{
    GLOBALDEBUG("[ChatManager::sendToRoomAll], roomId: " << iRoomId);

    vector<uint32_t> vUser = getRoomUser(iRoomId);

    GLOBALDEBUG("[ChatManager::sendToRoomAll], allUserCnt: " << vUser.size());
    
    for (auto m_iUid : vUser)
    {
        GLOBALDEBUG("[ChatManager::sendToRoomAll], now iUid: " << m_iUid);
        if(m_iUid != iUid)
        {
            SESSION_MANAGER->sendByUid(m_iUid, stCmdData);
        }
    }     
}


uint32_t ChatManager::changeNickName(uint32_t iUid, string sNewNickName)
{
    GLOBALINFO("[ChatManager::changeNickName], userId: " << iUid << " newNickName: " << sNewNickName);
    
    if(sNewNickName.empty() || sNewNickName.length() > 20)
    {
        return pb::ChangeNicknameRsp_Retcode_RET_FAIL;
    }

    // 开始更改db，update操作
    string execute = "update t_user set nickname = '" +sNewNickName + "' where uid = '" + 
    to_string(iUid) + "'";

    uint32_t iRet = SQL_MANAGER->dbUpdate(execute);
    if(iRet != 0)
    {
        GLOBALDEBUG("[ChatManager::changeNickName], update error");
        return pb::ChangeNicknameRsp_Retcode_RET_FAIL;
    }

    // notify 房间其他用户
    // 构造消息
    pb::NicknameChangeNotify rsp;
    auto names = getNameById(iUid);
    rsp.set_username(names[0]);
    rsp.set_nickname(names[1]);

    // NicknameChangeNotify -> stCmdData
    pb::CmdData stRsp;
    stRsp.set_cmdid(pb::CMD_CHANGE_NICKNAME_RSP); // 后面要改
    stRsp.set_cmddata(rsp.SerializeAsString());
    stRsp.set_ret(pb::CMD_CHANGE_NICKNAME_NOTIFY);

    // 发给房间内所有人，不包括自己(新版本)
    // if(m_mUid2Room.find(iUid) != m_mUid2Room.end())
    // {
    //     sendToRoomAll(iUid, m_mUid2Room[iUid], stRsp);    
    // }
    sendToRoomAll(iUid, m_mUid2Room[iUid], stRsp);
        
    return pb::ChangeNicknameRsp_Retcode_RET_SUCC;
}


bool ChatManager::checkRoomExist(uint32_t iRoomId)
{
    string query = "select rid from t_room where rid = '" + to_string(iRoomId) + "'";
    mysqlpp::StoreQueryResult res;
    uint32_t iRet = SQL_MANAGER->dbQuery(query, res);

    if(iRet != 0 || res.empty()) 
    {
        return false;
    }
    if(find(m_vRoom.begin(), m_vRoom.end(), iRoomId) == m_vRoom.end())
    {
        m_vRoom.push_back(iRoomId);
    }
    if(m_mRoom2Uid.find(iRoomId) == m_mRoom2Uid.end())
    {
        m_mRoom2Uid[iRoomId] = {};
    }
    return true;
}


void ChatManager::deleteMapByUserId(uint32_t iUid)
{
    GLOBALDEBUG("[ChatManager::deleteMapByUserId], iUid:" << iUid);
    auto it1 = m_mUid2Room.find(iUid);
    if(it1 != m_mUid2Room.end())
    {
        uint32_t iRoomId = it1->second;
        m_mUid2Room.erase(iUid);

        auto it2 = m_mRoom2Uid.find(iRoomId);
        if(it2 != m_mRoom2Uid.end())
        {
            auto itVec = find(it2->second.begin(), it2->second.end(), iUid);
            if(itVec != it2->second.end())
            {
                it2->second.erase(itVec);
            }
        }

    }


}
