#pragma once
#include <iostream>

#pragma comment(lib, "libmysql.lib")

#ifdef __linux__
#include <mysql/mysql.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <mysql.h>
#else
#error Not implemented!
#endif

/*
Класс, обеспечивающий создание/подключение к базе данных mySQL.
*/
class mySQL_API_database
{
public:
	mySQL_API_database(const std::string& host, const std::string& userName, const std::string& userPassword);	// Только инициализация полей класса
	~mySQL_API_database();	// Отключение от БД
	
	bool getStatus() const;
	std::string getName() const;
	MYSQL* getHandle();

	bool connect(const std::string& dbName);		// Подключение к существующей БД
	bool create(const std::string& dbName);			// Создание новой БД
	bool connectAnyway(const std::string& dbName);	// Подключение к существующей или новой БД

private:
	MYSQL _sqlHandle;
	bool _status;	// В работе не используется, можно исключить
	std::string _host;
	std::string _userName;
	std::string _userPassword;
	std::string _dbName;
};


/*
// Примеры использования:
mySQL_API_database test_db(constDB_host, constDB_user, constDB_password);

test_db.create(constDB_name1);
test_db.connect(constDB_name1);
...
test_db.connectAnyway(constDB_name2);
*/