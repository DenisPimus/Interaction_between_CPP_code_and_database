#include "../../include/Boost_Asio/Boost_Asio_client.h"



Boost_Asio_client::Boost_Asio_client(boost::asio::io_service* ioService): 
    _ioService(ioService), _socket(*_ioService), _started(true), _timer(*_ioService) { }

void Boost_Asio_client::doStart(boost::asio::ip::tcp::endpoint* ep) // Установление соединения с сервером
{
    _socket.async_connect(*ep, MEM_FN1(onConnect, boost::placeholders::_1));
}

void Boost_Asio_client::doStop()
{
    if (!_started) return;
    std::cout << "stopping " << _login << std::endl;
    _started = false;
    _socket.close();
}

bool Boost_Asio_client::isStarted()
{
    return _started;
}

void Boost_Asio_client::onConnect(const boost::system::error_code& err) // Реакция на успешное подключение - отправка данных аутентификации, формат: "register userLogin userPwHash userName eMail"
{
    if (!err)
    {
        char key;
        std::string userLogin, userPassword, userPwHash, userName, eMail;
        std::cout << "Приветствуем! Введите 1 для регистрации или 2 для входа: ";
        std::cin >> key;
        std::cin.ignore(1000, '\n');
        switch (key)
        {
        case '1':
            std::cout << "==Регистрация нового пользователя==" << std::endl << "Введите логин пользователя:"; // Строки не содержит пробелов - иначе нужно менять формат пересылки данных по сети (TODO)
            std::cin >> userLogin;
            std::cout << "Введите пароль:";
            std::cin >> userPassword;
            std::cout << "Введите имя:";
            std::cin >> userName;
            std::cout << "Введите адрес электронной почты:";
            std::cin >> eMail;
            userPwHash = sha1HEX(userPassword.c_str(), (uint)userPassword.length());
            userPwHash.erase(0, 1);
            //std::cout << "HEX: " << userPwHash << std::endl;
            doWrite("register " + userLogin + " " + userPwHash + " " + userName + " " + eMail + "\n");
            break;
        case '2':
            std::cout << "==Вход пользователя в чат==" << std::endl << "Введите логин пользователя:";
            std::cin >> userLogin;
            std::cout << "Введите пароль пользователя:";
            std::cin >> userPassword;
            userPwHash = sha1HEX(userPassword.c_str(), (uint)userPassword.length());
            userPwHash.erase(0, 1);
            doWrite("login " + userLogin + " " + userPwHash + "\n");
            break;
        default:
            std::cout << "Неверная команда. Выход." << std::endl;
            exit(3); //TODO: убрать костыль!
        }
    }
    else
        doStop();
}

void Boost_Asio_client::onRead(const boost::system::error_code& err, size_t bytes) // Обработка полученных данных после чтения
{
    if (err) doStop();
    if (!isStarted()) return;

    std::string msg(read_buffer_, bytes);

    if (msg.find("registration ") == 0)
        onRegistration(msg);
    else
        if (msg.find("login ") == 0)
            onLogin(msg);
        else
            if (msg.find("messages ") == 0)
                onNewMessages(msg);
            else
                if (msg.find("emptyMsgList") == 0)
                    onNoMessages();
                else
                    std::cerr << "Получен неверный ответ сервера:  " << msg << std::endl;
}

void Boost_Asio_client::onRegistration(const std::string& msg)
{
    std::istringstream in(msg);
    std::string tmp, answer;
    in >> tmp >> answer;
    if (answer == "ok")
    {
        std::cout << "Регистрация прошла успешно." << std::endl;
        boost::system::error_code ec;
        ec.assign(errno, boost::system::generic_category());
        onConnect(ec);
    }
    else
    {
        std::cout << "Ошибка регистрации!" << std::endl;
        boost::system::error_code ec;
        ec.assign(errno, boost::system::generic_category());
        onConnect(ec);
    }
}

void Boost_Asio_client::onLogin(const std::string& msg)   // Реакция на успешный вход
{
    std::istringstream in(msg);
    std::string tmp, answer, userLogin, userName;
    in >> tmp >> answer >> userLogin >> userName;
    if (answer == "ok")
    {
        std::cout << userName << ", Вы вошли в чат." << std::endl;
        _login = userLogin;
        _name = userName;
        doAskMessages();
    }
    else
    {
        std::cout << "Ошибка входа!" << std::endl;
        boost::system::error_code ec;
        ec.assign(errno, boost::system::generic_category());
        onConnect(ec);
    }
}

void Boost_Asio_client::onNewMessages(const std::string& msg)
{
    std::cout << "Получены новые сообщения:" << std::endl;
    std::string tmp, msgSender, msgText;
    std::istringstream in(msg);
    in >> tmp;
    while (in.rdbuf()->in_avail() > 0)
    {
        if(in >> msgSender >> msgText)
            std::cout << "[" << msgSender << "]: " << msgText << std::endl;
    }
    doSendMessage();
}

void Boost_Asio_client::onNoMessages()
{
    std::cout << "Для Вас нет новых сообщений." << std::endl;
    doSendMessage();
}

void Boost_Asio_client::onWrite(const boost::system::error_code& err, size_t bytes) // Действия после каждой отправки данных
{
    doRead();
}

void Boost_Asio_client::doRead()    // Чтение данных из сокета
{
    boost::asio::async_read(_socket, boost::asio::buffer(read_buffer_),
        MEM_FN2(readComplete, boost::placeholders::_1, boost::placeholders::_2), MEM_FN2(onRead, boost::placeholders::_1, boost::placeholders::_2));
}

void Boost_Asio_client::doWrite(const std::string& msg)
{
    if (!isStarted()) return;
    _writeBuffer = msg;
    _socket.async_write_some(boost::asio::buffer(_writeBuffer, _writeBuffer.size()),
        MEM_FN2(onWrite, boost::placeholders::_1, boost::placeholders::_2));
}

void Boost_Asio_client::doAskMessages()
{
    doWrite("ask_messages\n");
}

void Boost_Asio_client::doSendMessage()
{
    std::string messageReceiver, messageText, msg;
    std::cout << "==Отправка сообщения==" << std::endl << "Введите логин получателя:"; // Строки не содержит пробелов - иначе нужно менять формат пересылки данных по сети (TODO)
    std::cin >> messageReceiver;
    std::cout << "Введите сообщение {БЕЗ ПРОБЕЛОВ!}:";
    std::cin >> messageText;
    msg = "new_message " + messageReceiver + " " + messageText + "\n";
    doWrite(msg);
    doSendMessage();
}

size_t Boost_Asio_client::readComplete(const boost::system::error_code& err, size_t bytes)
{
    if (err) return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    return found ? 0 : 1;
}
