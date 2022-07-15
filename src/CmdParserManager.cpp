#include "CmdParserManager.h"
#include "LoginManager.h"
#include "ChatManager.h"
#include "LogManager.h"

uint32_t CmdParserManager::dealProtocal(uint32_t iUid, const string& sClientIP, const pb::CmdData& stReq, pb::CmdData& stRsp)
{
    try
    {
        switch (stReq.cmdid())
        {
            case pb::CMD_ECHO:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::ECHO]");

                    stRsp = stReq;
                }
                break;

            case pb::CMD_LOGIN_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::LOGIN_REQ]");

                    pb::LoginReq req;
                    pb::LoginRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRet = LOGIN_MANAGER->login(req.username(), req.passwd(), iUid);

                    GLOBALDEBUG("[case pb::CMD_LOGIN_REQ], iRet: " << iRet);
                   
                    // 开始组装rsp
                    rsp.set_usrid(iUid);
                    rsp.set_retcode((pb::LoginRsp_Retcode)iRet);

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_LOGIN_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                }
                break;

            case pb::CMD_REGISTER_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::REGISTER_REQ]");

                    pb::RegisterReq req;
                    pb::RegisterRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRet = LOGIN_MANAGER->registry(sClientIP, req.username(), 
                    req.nickname(), req.passwd());

                    GLOBALDEBUG("[case pb::CMD_REGISTER_REQ], iRet: " << iRet);

                    // 开始组装rsp
                    rsp.set_retcode((pb::RegisterRsp_Retcode)iRet);

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_REGISTER_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                }
                break;

            case pb::CMD_ENTER_ROOM_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::ENTER_ROOM_REQ]");

                    pb::EnterRoomReq req;
                    pb::EnterRoomRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRoomId = req.room_id();
                    vector<string> vUserName;
                    vector<pb::ChatMsg> vMsgList;
                    uint32_t iRet = CHAT_MANAGER->joinRoom(iUid, iRoomId, vUserName, vMsgList);

                    GLOBALDEBUG("[case pb::CMD_ENTER_ROOM_REQ], iRet: " << iRet);


                    if(iRet == pb::EnterRoomRsp_Retcode_RET_SUCC)
                    {
                        GLOBALDEBUG("[CmdParserManager::dealProtocal::ENTER_ROOM_REQ] success" <<" iUid: " << iUid);
                    }
                    else
                    {
                        GLOBALERROR("[CmdParserManager::dealProtocal::ENTER_ROOM_REQ] failed" <<" iUid: " << iUid);
                    }

                    // 开始组装rsp
                    rsp.set_room_id(iRoomId);
                    rsp.set_retcode((pb::EnterRoomRsp_Retcode)iRet);

                    for (auto name : vUserName)
                        rsp.add_username_list(name);
                   
                    pb::ChatMsg* addmsg = NULL;
                    for (auto msg : vMsgList)
                    {
                        addmsg = rsp.add_history_msg_list();
                        addmsg->CopyFrom(msg);
                    }

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_ENTER_ROOM_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                    stRsp.set_ret(iRet);
                }
                break;    

            case pb::CMD_LEAVE_ROOM_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::CMD_LEAVE_ROOM_REQ]");

                    pb::LeaveRoomReq req;
                    pb::LeaveRoomRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRet = CHAT_MANAGER->leaveRoom(iUid);

                    GLOBALDEBUG("[case pb::CMD_LEAVE_ROOM_REQ], iRet: " << iRet);

                    if(iRet == pb::LeaveRoomRsp_Retcode_RET_SUCC)
                    {
                        GLOBALDEBUG("[CmdParserManager::dealProtocal::LEAVE_ROOM_REQ] success" <<
                        " iUid: " << iUid << " msg: " << req.SerializeAsString());
                    }
                    else
                    {
                        GLOBALERROR("[CmdParserManager::dealProtocal::LEAVE_ROOM_REQ] failed" <<
                        " iUid: " << iUid << " msg: " << req.SerializeAsString() << " retCode: " << iRet);
                    }

                    // 开始组装rsp
                    rsp.set_retcode((pb::LeaveRoomRsp_Retcode)iRet);

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_LEAVE_ROOM_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                    stRsp.set_ret(iRet);
                }
                break;   

            case pb::CMD_SEND_MSG_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::CMD_SEND_MSG_REQ]");
                    
                    pb::SendMsgReq req;
                    pb::SendMsgRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRet = CHAT_MANAGER->sendChat(iUid, req);
                    
                    GLOBALDEBUG("[case pb::CMD_SEND_MSG_REQ], iRet: " << iRet);


                    if(iRet == pb::SendMsgRsp_Retcode::SendMsgRsp_Retcode_RET_SUCC)
                    {
                        GLOBALDEBUG("[CmdParserManager::dealProtocal::CMD_SEND_MSG_REQ] success" <<
                        " iUid: " << iUid << " msg: " << req.SerializeAsString());
                    } 
                    else
                    {
                        GLOBALERROR("[CmdParserManager::dealProtocal::CMD_SEND_MSG_REQ] failed" <<
                        " iUid: " << iUid << " msg: " << req.SerializeAsString());
                    }

                    // ToDo sendChat理念及其调用的函数需要继续丰富错误码
                    // 开始组装rsp
                    rsp.set_retcode((pb::SendMsgRsp_Retcode)iRet);

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_SEND_MSG_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                    stRsp.set_ret(iRet);
                }
                break;

            case pb::CMD_CHANGE_NICKNAME_REQ:
                {
                    GLOBALDEBUG("[CmdParserManager::dealProtocal::CMD_CHANGE_NICKNAME_REQ]");

                    pb::ChangeNicknameReq req;
                    pb::ChangeNicknameRsp rsp;

                    req.ParseFromString(stReq.cmddata());

                    uint32_t iRet = 0;
                    iRet = CHAT_MANAGER->changeNickName(iUid, req.nickname());

                    GLOBALDEBUG("[case pb::CMD_CHANGE_NICKNAME_REQ], iRet: " << iRet);

                    // rsp -> stRsp
                    stRsp.set_cmdid(pb::CMD_CHANGE_NICKNAME_RSP);
                    stRsp.set_cmddata(rsp.SerializeAsString());
                    stRsp.set_ret(iRet);
                }
                break;

            

            default:
                return pb::ErrorCode_Net_ProtocalInvalid;
        }
    }
    catch(const std::exception& e)
    {
        return pb::ErrorCode_System_Exception;
    }

    return 0;
}
