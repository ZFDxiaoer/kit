#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>

#include "cmd.pb.h"
#include <deque>

using boost::asio::ip::tcp;
using namespace std;

#define BUFFER_MAX_LENGTH 1024
// #define PROTO_MAX_LENGTH 100000
#define PROTO_MAX_LENGTH 80000
#define PROTO_LENGTH_BYTES_NUM 4

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(tcp::socket socket);

        void start();
        void disConnect();
        void send(const pb::CmdData&);

        // add
        static uint32_t iUidCnt;

    private:
        uint32_t calcProtocalLength(uint32_t&);
        void dealProtocalData();
        void sendErrorCmd(uint32_t err);

        void do_read();
        void do_write();

        

    private:
        static uint32_t bytes2uint32(char* vBytes, uint32_t size = 4);

        tcp::socket socket_;

        char        m_vReadBuffer[BUFFER_MAX_LENGTH];
        char        m_vWriteBuffer[BUFFER_MAX_LENGTH];

        bool        m_bStartRecv;
        uint32_t    m_iProtocalBytes;
        char        m_vRecieveBytes[PROTO_MAX_LENGTH + BUFFER_MAX_LENGTH + 100];
        uint32_t    m_iRecieveBytesNum;

        deque<pb::CmdData> m_qSendCmds;
        
        uint32_t    m_iUid;
        string      m_sClientIP;
};



#endif
