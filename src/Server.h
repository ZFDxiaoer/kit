#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server
{
    public:
        Server(boost::asio::io_context& io_context, short port);

        uint32_t init();
        void run();

    private:
        void do_accept();

        tcp::acceptor acceptor_;
};

#endif
