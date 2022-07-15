#include "Session.h"
#include "SessionManager.h"
#include "CmdParserManager.h"
#include "LogManager.h"
#include "LoginManager.h"

#include <thread>

using namespace std;

Session::Session(tcp::socket socket) : socket_(std::move(socket)) 
{
    m_bStartRecv = false;
    m_iProtocalBytes = 0;
    m_iRecieveBytesNum = 0;
    m_sClientIP = socket_.remote_endpoint().address().to_string();
    m_iUid = 0;
}

void Session::start()
{
    do_read();
}

void Session::disConnect()
{
    SESSION_MANAGER->onDisConnect(m_iUid);

    boost::system::error_code ignored_ec;  
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void Session::send(const pb::CmdData& stSendCmd)
{
    GLOBALDEBUG("[Session::send]thread id:" << this_thread::get_id() << ", begin send cmd:" << stSendCmd.cmdid());
    bool write_in_progress = !m_qSendCmds.empty();
    m_qSendCmds.push_back(stSendCmd);            

    GLOBALDEBUG("[Session::send]thread id:" << this_thread::get_id() << ", cmd size:" << m_qSendCmds.size() << ", new write:" << (write_in_progress ? 1 : 0));
    if(!write_in_progress)
    {
        do_write();
    }
}

uint32_t Session::calcProtocalLength(uint32_t& iProtocalLength)
{
    try
    {
        if (m_iRecieveBytesNum < PROTO_LENGTH_BYTES_NUM)
        {
            return pb::ErrorCode_Net_ProtocalLength;
        }

        iProtocalLength = bytes2uint32(m_vRecieveBytes, PROTO_LENGTH_BYTES_NUM);

        // 数据包过长直接断开session
        if (iProtocalLength >= PROTO_MAX_LENGTH)
        {
            GLOBALDEBUG("[Session::calcProtocalLength] protocal is too long, length is " << iProtocalLength);
            return pb::ErrorCode_Net_ProtocalLengthMax;
        }
    }
    catch(std::exception& e)
    {
        return pb::ErrorCode_System_Exception;
    }
    return 0;
}

void Session::dealProtocalData()
{
    try
    {
        GLOBALDEBUG("[Session::dealProtocalData]thread id:" << this_thread::get_id() << ", begin deal new");

        pb::CmdData stReq;
        stReq.ParseFromArray(m_vRecieveBytes + PROTO_LENGTH_BYTES_NUM, m_iProtocalBytes - PROTO_LENGTH_BYTES_NUM);


        // 第一条协议必须是登录协议
        GLOBALDEBUG("[Session::dealProtocalData], before, iUid: " << m_iUid);

        if (stReq.cmdid() != pb::CMD_ECHO 
            && m_iUid == 0 
            && stReq.cmdid() != pb::CMD_LOGIN_REQ
            && stReq.cmdid() != pb::CMD_REGISTER_REQ)
        {
            sendErrorCmd(pb::ErrorCode_Net_ProtocalInvalid);
            return;
        }

        GLOBALDEBUG("[Session::dealProtocalData]thread id:" << this_thread::get_id() << ", begin deal cmd req:" << stReq.cmdid());

        // 处理协议
        pb::CmdData stRsp;
        uint32_t iRet = CMD_PARSER_MANAGER->dealProtocal(m_iUid, m_sClientIP, stReq, stRsp);
        if (iRet != 0)
        {
            sendErrorCmd(iRet);
            return;
        }

        GLOBALDEBUG("[Session::dealProtocalData]thread id:" << this_thread::get_id() << ", end deal cmd rsp:" << stRsp.cmdid());

        // 特殊协议额外处理
        // 更新本session所属用户
        if (stRsp.cmdid() == pb::CMD_LOGIN_RSP)
        {
            pb::LoginRsp stLoginRsp;
            stLoginRsp.ParseFromString(stRsp.cmddata());
            m_iUid = stLoginRsp.usrid();
            SESSION_MANAGER->onConnect(m_iUid, shared_from_this());
        }

        GLOBALDEBUG("[Session::dealProtocalData], after, iUid: " << m_iUid);

        // 回复
        send(stRsp);
    }
    catch(std::exception& e)
    {
        sendErrorCmd(pb::ErrorCode_Net_ProtocalInvalid);
    }
}

void Session::sendErrorCmd(uint32_t err)
{
    pb::CmdData stError;
    stError.set_ret(err);

    send(stError);
}

void Session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(m_vReadBuffer, BUFFER_MAX_LENGTH),
                            [this, self](boost::system::error_code ec, std::size_t length)
    {
        if (ec)
        {
            disConnect();
            return;
        }

        if (length != 0)
        {
            // 收到了新的数据，处理这批数据，不会大于最大长度，因为最开始超过最大长度直接断开链接了。
            memcpy(m_vRecieveBytes + m_iRecieveBytesNum, m_vReadBuffer, length);
            m_iRecieveBytesNum += length;
            do
            {
                if (!m_bStartRecv)
                {
                    // 计算本次协议的长度
                    uint32_t iRet = calcProtocalLength(m_iProtocalBytes);
                    if (iRet == pb::ErrorCode_Net_ProtocalLength)
                    {
                        // 收到的数据解析不出长度，继续读
                        break;
                    }

                    // 比如数据过长
                    else if (iRet != 0)
                    {
                        disConnect();
                        return;
                    }

                    m_bStartRecv = true;
                }

                if (m_iRecieveBytesNum >= m_iProtocalBytes)
                {
                    // 接受到了一个完整的协议
                    dealProtocalData();

                    m_iRecieveBytesNum -= m_iProtocalBytes;
                    memmove(m_vRecieveBytes, m_vRecieveBytes + m_iProtocalBytes, m_iRecieveBytesNum);

                    m_bStartRecv = false;
                    m_iProtocalBytes = 0;
                }
                else
                {
                    // 目前接受到的不够，继续读
                    break;
                }
            }
            while (true);
        }

        // 继续读
        do_read();
    });
}

void Session::do_write()
{
    GLOBALDEBUG("[Session::do_write]thread id:" << this_thread::get_id() << ", enter");
    if (m_qSendCmds.empty())
    {
        GLOBALDEBUG("[Session::do_write]thread id:" << this_thread::get_id() << ", empty return");
        return;
    }
    
    const pb::CmdData& stSendCmd = m_qSendCmds.front();

    int32_t size = stSendCmd.ByteSizeLong();
    uint32_t iTotalLength = 4 + size;

    if (iTotalLength >= PROTO_MAX_LENGTH)
    {
        GLOBALERROR("[Session::do_write]send response too long:" << stSendCmd.cmdid());
        disConnect();
        return;
    }

    uint32_t iNetTotalLength = htonl(iTotalLength);
    memcpy(m_vWriteBuffer, &iNetTotalLength, 4);
    stSendCmd.SerializeToArray(m_vWriteBuffer + 4, size);

    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(m_vWriteBuffer, iTotalLength),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
    {
        if (ec)
        {
            disConnect();
            return;
        }

        m_qSendCmds.pop_front();
        GLOBALDEBUG("[Session::do_write]thread id:" << this_thread::get_id() << ", callback, cmd size:" << m_qSendCmds.size());

        if(!m_qSendCmds.empty()) {
            do_write();
        }
    });
}

uint32_t Session::bytes2uint32(char* vBytes, uint32_t size)
{
    if (size > 4)
    {
        size = 4;
    }

    uint32_t iRetVal = 0;
    uint32_t iOffset = sizeof(uint32_t) - size;
    memcpy(&iRetVal + iOffset, vBytes, size);
    iRetVal = ntohl(iRetVal);
    return iRetVal;
}  
