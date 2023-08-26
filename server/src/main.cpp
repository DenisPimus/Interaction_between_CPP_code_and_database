#include <iostream>
#include "../include/util/utsname.h"
#include "../include/mySQL_API/mySQL_API_database.h"
#include "../include/mySQL_API/mySQL_API_usersTable.h"
#include "../include/mySQL_API/mySQL_API_messagesTable.h"

#include "../include/Boost_Asio/Boost_Asio_server.h"
constexpr boost::asio::ip::port_type serverPortNumber = 5190; // The King is dead. Long live the King!

constexpr const char* chatVersion = "v0.6";

constexpr const char* constDB_host = "localhost";
constexpr const char* constDB_name = "chat";
constexpr const char* constDB_user = "user";
constexpr const char* constDB_password = "password";

boost::asio::io_service ioService;
boost::asio::ip::tcp::endpoint _listenEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverPortNumber);		// Endpoint(IPv4, PORT)
boost::asio::ip::tcp::acceptor _listenAcceptor = boost::asio::ip::tcp::acceptor(ioService, _listenEndpoint);						// Acceptor(ioservice, Endpoint)

void onClientConnected(boost::shared_ptr <Boost_Asio_server> client, boost::asio::io_service* _ioService, const boost::system::error_code& err)
{
	client->onStart();	// Новый клиент подключился - запускаем обработку события, ->
	boost::shared_ptr <Boost_Asio_server> new_client(new Boost_Asio_server(client.get())); // после чего открываем сокет для другого возможного клиента и ->
	_listenAcceptor.async_accept(new_client->getSocket(), boost::bind(onClientConnected, new_client, _ioService, boost::placeholders::_1)); // создаём новый поток для обработки нового возможного подключения
}

int main() 
{
	setlocale(LC_ALL, "");

	struct utsname utsname;
	uname(&utsname);
	std::cout << "Chat server (" << chatVersion << "-" << utsname.machine << ") running on '" << utsname.nodename << "' under "
		<< utsname.sysname << " " << utsname.release << " build " << utsname.version << std::endl;

	boost::shared_ptr <Boost_Asio_server> firstClient(new Boost_Asio_server(&ioService, constDB_host, constDB_user, constDB_password, constDB_name)); // Инициализируем первого клиента ->
	_listenAcceptor.async_accept(firstClient->getSocket(), boost::bind(onClientConnected, firstClient, &ioService, boost::placeholders::_1)); // и запускаем ожидание его подключения 
	ioService.run(); // Дальше исполнение не идёт 


	mySQL_API_database chatDB(constDB_host, constDB_user, constDB_password);
	chatDB.connectAnyway(constDB_name);
	mySQL_API_usersTable chatUsers(&chatDB);
	mySQL_API_messagesTable chatMessages(&chatDB);

	chatUsers.addUser("user1", "HASH1234567890", "name1", "test@test.ru");
	chatUsers.addUser("user2", "pass2", "name2", "test2@test.ru");
	chatUsers.addUser("user3", "pass3", "name3", "test3@test.ru");

	chatUsers.login("user1", "HASH1234567890");
	chatUsers.login("user2", "pass2");
	chatUsers.login("user3", "wrong");

	chatMessages.addMessage("1", "2", "1-2");
	chatMessages.addMessage("2", "3", "2-3");
	chatMessages.addMessage("3", "1", "3-1");
	chatMessages.addMessage("1", "3", "1-3");
	chatMessages.addMessage("2", "1", "2-1");
	chatMessages.addMessage("3", "2", "3-2");

	chatMessages.setRecievedMultiple(chatMessages.getUnreceivedMessagesIdsForUserId("1"));


	chatUsers.logout("user1");
	chatUsers.deleteUser("user2");



	return 0;
}