#pragma once
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/core/noncopyable.hpp>
#include <iostream>
#include "../../include/util/sha1.h"

class Boost_Asio_client;
#define MEM_FN(x)       boost::bind(&Boost_Asio_client::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&Boost_Asio_client::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&Boost_Asio_client::x, shared_from_this(),y,z)

constexpr unsigned int pingInterval_client = 2; // Интервал между уведомлениями сервера о статусе, мс


class Boost_Asio_client : public boost::enable_shared_from_this<Boost_Asio_client>// , boost::noncopyable 
{
public:
    Boost_Asio_client(boost::asio::io_service* ioService);

    void doStart(boost::asio::ip::tcp::endpoint* ep);                   // Установление соединения с сервером  
    void doStop();                                                      // Действия при отключении клиента
    
    bool isStarted();

private:
    boost::asio::io_service* _ioService;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _timer;
    bool _started;
    std::string _login;
    std::string _name;

    enum { max_msg = 1024 };
    char read_buffer_[max_msg];		// TODO: Заменить на std::string!
    std::string _writeBuffer;

    void onConnect(const boost::system::error_code& err);               // Реакция на успешное подключение - отправка данных аутентификации
    void onRead(const boost::system::error_code& err, size_t bytes);    // Обработка полученных данных после чтения

    void onRegistration(const std::string& msg);                                              // Реакция на попытку регистрации
    void onLogin(const std::string& msg);                                                     // Реакция на попытку входа
    void onNewMessages(const std::string& msg);
    void onNoMessages();

    void onWrite(const boost::system::error_code& err, size_t bytes);   // Действия после каждой отправки данных

    void doRead();                                                      // Чтение данных из сокета
    void doWrite(const std::string& msg);                               // Отправка данных в сокет
    void doAskMessages();                                               // Отправка запроса на чтение новых сообщений
    void doSendMessage();                                               // Отправка сообщения

    size_t readComplete(const boost::system::error_code& err, size_t bytes);    // Проверка завершения чтения

};

// Доступные команды от пользователя: 
// login name name -> переделать на userLogin userPwHash userName eMail - CHECK? - OK
// ping
// ask_clients
// 
// Добавить:
// register userLogin userPwHash userName eMail - CHECK? - OK
// send_message reciever, message: addMessage(_login, reciever, message);
// ask_messages: doWrite( messages.getUnreceivedMessagesIdsForUserId(getIdByLogin(_login)) )
//
// Доступные ответы сервера:
// login ok (onLogin)
// ping client_list_changed (onPing)
// ping ok (onPing)
// clients u1, u2,... (onClients)
//
