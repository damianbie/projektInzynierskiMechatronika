#include "server.hpp"
#include "esp_log.h"
#include "System.hpp"

server::server(asio::io_context& io_context, short port)
    :acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    _clientNextId(1000)
{
    acceptConnection();
}
void server::acceptConnection()
{
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                auto client = std::make_shared<con>(std::move(socket), this->_clientNextId++);
                ESP_LOGI("TCP_SER", "Client connected id->%i, allCliets->%i", client->getID(), this->clients.size());
                this->readOnClient(client);
                this->clients.push_back(client);
            }
            acceptConnection();
        });
}
void server::readOnClient(std::shared_ptr<con> cl)
{
    cl->getSocket().async_read_some(asio::buffer(cl->buff, 1024),
    [this, cl](std::error_code er, std::size_t len) -> void{
        if(!er) 
        {
            cl->buff[len] = '\0';

            std::string cmd(cl->buff);
            auto result = this->parseCommand(cmd);
            // ESP_LOGW("TCP_SERV", "CLIENT[%i]: %s, parametry -> %i", cl->getID(), cl->buff, result.second.size());

            std::string cmdRes = System::getInstance().runCommand(result, cl, *this);
            if(cmdRes.length())
            {
                asio::write(cl->getSocket(), asio::buffer(cmdRes, cmdRes.length()));
            }
            this->readOnClient(cl);
            cl->buff[0] = '\0';
        }
        else
        {
            this->close(cl);
        }
    });
}
void server::close(std::shared_ptr<con> cl)
{
    auto ind = std::find(this->clients.begin(), this->clients.end(), cl);
    if(ind != this->clients.end())
    {
        ESP_LOGW("TCP_SERV", "CLIENT[%i]: Disconnected", cl->getID());
        cl->getSocket().close();
        this->clients.erase(ind);
    }
}
returnCommandType server::parseCommand(std::string cmd)
{
    std::string s, cmdName;
    std::vector<std::string> args;
    while (cmd.length())
	{
		int ind = cmd.find_first_of(' ');
		if (ind == -1) ind = cmd.length();

		s = cmd.substr(0, ind);
		if (s.empty())
			cmd.erase(cmd.begin(), cmd.begin() + 1);
		else
		{
            if(cmdName.empty())
                cmdName = s;
            else
                args.push_back(s);

			cmd.erase(cmd.begin(), cmd.begin() + ind);
		}
	}
    return std::make_pair(cmdName, args);
}