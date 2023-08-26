#pragma once
#include "mySQL_API_table.h"
#include "mySQL_API_consts.h"

/*
Класс, обеспечивающий работу с таблицей пользователей. 
Список полей (определены в mySQL_API_consts.h):
constUsr_id = "id";	- первичный ключ
constUsr_login = "login";	- логин
constUsr_hash = "pwHash";	- хеш пароля
constUsr_name = "name";	- "гражданское" имя
constUsr_regDate = "regDate";	- время регистрации в чате
constUsr_eMail = "email";	- адрес электронной почты
constUsr_status = "status";	- статус: offline, online, deleted(данные пользователя, кроме пароля, остаются в базе, но вход запрещён)
constUsr_lastLogin = "lastLogin";	- время, обновляется при входе и выходе из чата
Класс обеспечивает добавление/удаление пользователей, вход(с проверкой данных авторизации)/выход, чтение/изменение ряда столбцов
*/
class mySQL_API_usersTable : public mySQL_API_table
{
public:
	mySQL_API_usersTable(mySQL_API_database* dbHandle);	// Вызывает конструктор родительского класса, который создаёт пустую таблицу при её отсутствии в БД
	~mySQL_API_usersTable();	//	Изменение статуса всех подключенных пользователей на offline

	std::string getIdByLogin(const std::string& userLogin);
	const std::string getLoginById(const std::string& userId);
	const std::string getPwHashById(const std::string& userId);
	const std::string getNameById(const std::string& userId);
	const std::string getRegDateById(const std::string& userId);
	const std::string getEMailById(const std::string& userId);
	const std::string getStatusById(const std::string& userId);
	void setOnline(const std::string& userId);
	void setOffline(const std::string& userId);
	bool isOnline(const std::string& userId);
	bool isOffline(const std::string& userId);
	bool isDeleted(const std::string& userId);

	bool addUser(const std::string& userLogin, const std::string& userPwHash, const std::string& userName, const std::string& eMail );	// Добавление пользователя с проверкой на дублирование
	bool deleteUser(const std::string& userLogin);	// Изменение статуса (делает невозможным дальнейший вход в систему)
	bool login(const std::string& userLogin, const std::string& userPwHash);	// Вход в систему с проверкой по статусу и хешу пароля
	bool logout(const std::string& userLogin);	// Изменение статуса

private:

};


/*
// Примеры использования:
mySQL_API_database test_db(constDB_host, constDB_user, constDB_password);
test_db.connectAnyway(constDB_name);

mySQL_API_usersTable users(&test_db);

users.addUser("user1", "HASH1234567890",	"name1", "test@test.ru");
users.addUser("user2", "pass2",				"name2", "test2@test.ru");
users.addUser("user3", "pass3",				"name3", "test3@test.ru");

users.login("user1", "HASH1234567890");
users.login("user2", "pass2");
users.login("user3", "wrong");

users.logout("user1");
users.deleteUser("user2");
*/