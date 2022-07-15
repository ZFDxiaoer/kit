#include "Server.h"
#include "Session.h"
#include "LogManager.h"

Server::Server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{}

uint32_t Server::init()
{
    if(LogManager::init() != 0) 
    {
        // ToDo error, Init error
        return 1;
    }

    GLOBALINFO("Server Start...");

    return 0;
}

void Server::run()
{
    do_accept();
}


void Server::do_accept()
{
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket)
    {
        if (!ec)
        {
            std::make_shared<Session>(std::move(socket))->start();
        }
        else
        {
            GLOBALERROR("sock accept error:" << ec.message());
        }

        do_accept();
    });
}

