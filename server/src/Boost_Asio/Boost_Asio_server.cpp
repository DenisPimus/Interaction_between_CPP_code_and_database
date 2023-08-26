#include "../../include/Boost_Asio/Boost_Asio_server.h"

Boost_Asio_server::Boost_Asio_server (boost::asio::io_service* ioService, const std::string& host, const std::string& user, const std::string& password, const std::string& dbName) :
    _ioService(ioService), _socket(*ioService), _timer(*ioService), _started(false) 
{ 
    chatDB = new mySQL_API_database(host, user, password);
    chatDB->connectAnyway(dbName);
    chatUsers = new mySQL_API_usersTable(chatDB);
    chatMessages = new mySQL_API_messagesTable(chatDB);
}

Boost_Asio_server::Boost_Asio_server(Boost_Asio_server* parent) :
    _ioService(parent->_ioService), _socket(*parent->_ioService), _timer(*parent->_ioService), _started(false)
{
    chatDB = parent->chatDB;
    chatUsers = parent->chatUsers;
    chatMessages = parent->chatMessages;
}

Boost_Asio_server::~Boost_Asio_server()
{
    delete chatMessages;
    delete chatUsers;
    delete chatDB;
}

void Boost_Asio_server::onStart() // Действия после подключения клиента к порту
{
    _started = true;
    clientsArray.push_back(shared_from_this());
    doRead();   // После подключения клиент должен первым прислать сообщение с запросом авторизации - ждём
}

void Boost_Asio_server::onStop()   // Действия при отключении клиента
{
    if (!_started) return;
    chatUsers->logout(_login);
    _started = false;
    _socket.close();
    boost::shared_ptr<Boost_Asio_server> ptrToSelf = shared_from_this();
    clientsArrayType::iterator iterToSelf = std::find(clientsArray.begin(), clientsArray.end(), ptrToSelf);
    clientsArray.erase(iterToSelf);
}

bool Boost_Asio_server::isStarted() const
{
    return _started;
}

boost::asio::ip::tcp::socket& Boost_Asio_server::getSocket()
{
    return _socket;
}

std::string Boost_Asio_server::getLogin() const
{
    return _login;
}

void Boost_Asio_server::onRead(const boost::system::error_code& err, size_t bytes)   // Обработка полученных данных по-окончании чтения
{
    if (err) onStop();
    if (!isStarted()) return;

    std::string msg(read_buffer_, bytes);

    // Ищем в сообщении клиента запрос:
    if (msg.find("register ") == 0) 
        onRegister(msg);                                                    // Регистрация нового пользователя
    else 
        if (msg.find("login ") == 0) 
            onLogin(msg);                                                   // Авторизация клиента
        else 
            if (msg.find("ask_clients") == 0)
                onClients();                                                // Запрос о списке пользователей
            else
                if (msg.find("ask_messages") == 0)
                    onGetMessages();                                        // Запрос о новых сообщениях
                else
                    if (msg.find("new_message") == 0)
                        onNewMessage(msg);                                  // Запрос о новых сообщениях
                    else
                        std::cerr << "Получен неверный запрос клиента: " << msg << std::endl;
}

void Boost_Asio_server::onWrite(const boost::system::error_code& err, size_t bytes)  // Действия по-окончании отправки данных
{
    doRead();   // Читаем ответ клиента
}

void Boost_Asio_server::onRegister(const std::string& msg) // Реакция на запрос о регистрации, формат: "register userLogin userPwHash userName eMail"
{
    std::string tmp, userLogin, userPwHash, userName, eMail;
    std::istringstream in(msg);
    in >> tmp >> userLogin >> userPwHash >> userName >> eMail;
    if (chatUsers->addUser(userLogin, userPwHash, userName, eMail))
    {
        std::cout << userLogin << " зарегистрирован." << std::endl;
        doWrite("registration ok\n");
    }
    else
    {
        std::cout << userLogin << " не зарегистрирован!" << std::endl;
        doWrite("registration failed\n");
    }    
}

void Boost_Asio_server::onLogin(const std::string& msg) // Реакция на запрос о входе в чат, формат: "login userLogin userPwHash"
{
    std::string tmp, userLogin, userPwHash, userName;
    std::istringstream in(msg);
    in >> tmp >> userLogin >> userPwHash;
    if (chatUsers->login(userLogin, userPwHash))
    {
        std::cout << userLogin << " вошёл в систему" << std::endl;
        _login = userLogin;
        userName = chatUsers->getNameById(chatUsers->getIdByLogin(userLogin));
        doWrite("login ok " + userLogin + ' ' + userName + '\n');
    }
    else
    {
        std::cout << userLogin << " - ошибка входа!" << std::endl;
        _login.clear();
        doWrite("login failed\n");
    }
}

void Boost_Asio_server::onClients() // Реакция на запрос о списке онлайн-пользователей
{
    std::string msg;
    for (clientsArrayType::const_iterator b = clientsArray.begin(), e = clientsArray.end(); b != e; ++b)
        msg += (*b)->getLogin() + " ";
    doWrite("clients " + msg + "\n");
}

void Boost_Asio_server::onGetMessages()
{
    std::vector<std::string> allMessages = chatMessages->getUnreceivedMessagesIdsForUserId(chatUsers->getIdByLogin(_login));
    std::string msgSender, msgText, result;
    for (auto i = allMessages.begin(); i < allMessages.end(); ++i)
    {
        msgSender = chatMessages->getSenderById(*i);
        msgSender = chatUsers->getLoginById(msgSender);
        msgText = chatMessages->getTextById(*i);
        result += msgSender + " " + msgText + " ";
        chatMessages->setReceived(*i);
    }
    if (allMessages.begin() != allMessages.end())
        doWrite("messages " + result + "\n");
    else
        doWrite("emptyMsgList \n");
}

void Boost_Asio_server::onNewMessage(const std::string& msg)
{
    std::string tmp, msgReceiver, msgText;
    std::istringstream in(msg);
    in >> tmp >> msgReceiver >> msgText;
    chatMessages->addMessage(chatUsers->getIdByLogin(_login), chatUsers->getIdByLogin(msgReceiver), msgText);
}

void Boost_Asio_server::doRead() // Чтение данных из сокета _socket в буфер read_buffer_
{
    boost::asio::async_read(_socket, 
        boost::asio::buffer(read_buffer_),
        MEM_FN2(readComplete, boost::placeholders::_1, boost::placeholders::_2),
        MEM_FN2(onRead, boost::placeholders::_1, boost::placeholders::_2));      
}

void Boost_Asio_server::doWrite(const std::string& msg) // Отправка данных в сокет
{
    if (!isStarted()) return;
    _writeBuffer = msg;
    _socket.async_write_some(boost::asio::buffer(_writeBuffer, _writeBuffer.size()),
        MEM_FN2(onWrite, boost::placeholders::_1, boost::placeholders::_2));
}

size_t Boost_Asio_server::readComplete(const boost::system::error_code& err, size_t bytes)   // Проверка завершения чтения
{
    if (err) return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}
