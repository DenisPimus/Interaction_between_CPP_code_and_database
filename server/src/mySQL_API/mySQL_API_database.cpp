#include "../../include/mySQL_API/mySQL_API_database.h"

mySQL_API_database::mySQL_API_database(const std::string& host, const std::string& userName, const std::string& userPassword) : 
																_host(host), _userName(userName), _userPassword(userPassword), _status(false) 
{ 

}

mySQL_API_database::~mySQL_API_database()
{
	mysql_close(&_sqlHandle);
	_status = false;
}

bool mySQL_API_database::getStatus() const
{
	return _status;
}

std::string mySQL_API_database::getName() const
{
	return _dbName;
}

MYSQL* mySQL_API_database::getHandle() 
{
	return &_sqlHandle;
}

bool mySQL_API_database::connect(const std::string& dbName)
{
	_dbName = dbName;

	mysql_init(&_sqlHandle);
	if (&_sqlHandle == nullptr) {
		// Если дескриптор не получен — выводим сообщение об ошибке
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_database::connect> Can't create MySQL-descriptor" << std::endl;
#endif 
		_status = false;
		return _status;
	}

	// Подключаемся к серверу
	if (!mysql_real_connect(&_sqlHandle, _host.c_str(), _userName.c_str(), _userPassword.c_str(), _dbName.c_str(), NULL, NULL, 0)) {
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_database::connect> " << mysql_error(&_sqlHandle) << std::endl;
#endif 
		_status = false;
		return _status;
	}
	else {
		// Если соединение успешно установлено выводим фразу — "Success!"
#ifdef _DEBUG
		if(!_dbName.empty()) // Не информируем о подключении к "пустой" БД для создания новой
			std::cout << "[  OK  ] <mySQL_API_database::connect> Connected to " << _dbName << std::endl;
#endif 
		_status = true;
	}
	mysql_set_character_set(&_sqlHandle, "utf8mb4"); // В модуле было utf8==utf8mb3 (хранит только 3 байта), а нам нужны будут смайлики - зарепортить!

	return true;
}

bool mySQL_API_database::create(const std::string& dbName)
{
	std::string req = "CREATE DATABASE IF NOT EXISTS " + dbName;
	if (connect(""))
	{
		if (mysql_query(&_sqlHandle, req.c_str()))
			return true;
	}
	return false;
}

bool mySQL_API_database::connectAnyway(const std::string& dbName)
{
	create(dbName);
	return connect(dbName);
}
