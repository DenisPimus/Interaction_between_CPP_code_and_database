#pragma once

#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "../mySQL_API/mySQL_API_database.h"
#include "../mySQL_API/mySQL_API_usersTable.h"
#include "../mySQL_API/mySQL_API_messagesTable.h"

constexpr unsigned int pingInterval_server = 10000; // Интервал максимального отсутствия связи с клиентом, мс



class Boost_Asio_server;
typedef std::vector< boost::shared_ptr< Boost_Asio_server > > clientsArrayType;	// Определение типа для хранения списка подключенных клиентов
clientsArrayType clientsArray; // Глобальная переменная для хранения полного списка клиентов, с которыми в конкретный момент времени установлена связь 
//TODO: добавить семафор на одновременный доступ из ::onStart, ::onStop, ::onClients

#define MEM_FN(x)       boost::bind(&Boost_Asio_server::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&Boost_Asio_server::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&Boost_Asio_server::x, shared_from_this(),y,z)

class Boost_Asio_server : public boost::enable_shared_from_this<Boost_Asio_server> //, boost::noncopyable
{
public:
	Boost_Asio_server(boost::asio::io_service* ioService, const std::string& host, const std::string& user, const std::string& password, const std::string& dbName);
	Boost_Asio_server(boost::asio::io_service* ioService);
	Boost_Asio_server(Boost_Asio_server* parent);
	~Boost_Asio_server();	// Как в него попасть до завершения приложения???
	void onStart();														// Действия после подключения клиента к порту
	void onStop();														// Действия при отключении клиента

	bool							isStarted() const;
	boost::asio::ip::tcp::socket&	getSocket();
	std::string						getLogin() const;
	void							setClientsChanged();

private:
	boost::asio::io_service* _ioService;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::deadline_timer _timer;
	bool _started;
	std::string _login;

	enum { max_msg = 1024 };
	char read_buffer_[max_msg];		// TODO: Заменить на std::string!
	std::string _writeBuffer;

	void onRead(const boost::system::error_code& err, size_t bytes);	// Обработка полученных данных после чтения
	void onWrite(const boost::system::error_code& err, size_t bytes);	// Действия после каждой отправки данных
	void onRegister(const std::string& msg);								// Реакция на запрос о входе в чат TODO: проверка логина и пароля по базе пользователей
	void onLogin(const std::string& msg);								// Реакция на запрос о входе в чат TODO: проверка логина и пароля по базе пользователей
	void onClients();													// Реакция на запрос о списке онлайн-пользователей
	void onGetMessages();												// Реакция на запрос новых сообщений
	void onNewMessage(const std::string& msg );							// Реакция на получение нового сообщения


	void doRead();														// Чтение данных из сокета
	void doWrite(const std::string& msg);								// Отправка данных в сокет
	
	size_t readComplete(const boost::system::error_code& err, size_t bytes); // Проверка завершения чтения

	mySQL_API_database* chatDB;
	mySQL_API_usersTable* chatUsers;
	mySQL_API_messagesTable* chatMessages;
};

// Доступные команды от пользователя: 
// login name name -> переделать на userLogin userPwHash userName eMail - CHECK?
// ping
// ask_clients
// 
// Добавить:
// register userLogin userPwHash userName eMail - CHECK?
// send_message reciever, message: addMessage(_login, reciever, message);
// read_messages: doWrite( messages.getUnreceivedMessagesIdsForUserId(getIdByLogin(_login)) )
//
// Доступные ответы сервера:
// login ok (onLogin)
// ping client_list_changed (onPing)
// ping ok (onPing)
// clients u1, u2,... (onClients)
//


