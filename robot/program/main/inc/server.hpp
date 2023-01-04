#ifndef _H_SERVER_H
#define _H_SERVER_H

#include <memory>
#include <utility>
#include "asio.hpp"

using asio::ip::tcp;

typedef std::pair<std::string, std::vector<std::string>> returnCommandType;

class con
{
  public:
  con(tcp::socket s, int i)
  :_soc(std::move(s)), id(i)
  { };

  tcp::socket& getSocket()
  {
    return this->_soc;
  }
  int getID(){return this->id;}

  char buff[1024];
  private:
  tcp::socket _soc;
  int id;
};

class server
{
  public:
    server(asio::io_context& io_context, short port);

    void close(std::shared_ptr<con> cl);
  protected:
    returnCommandType parseCommand(std::string);
    void acceptConnection();
    void readOnClient(std::shared_ptr<con>);

    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<con>> clients;
    int _clientNextId;
};
#endif