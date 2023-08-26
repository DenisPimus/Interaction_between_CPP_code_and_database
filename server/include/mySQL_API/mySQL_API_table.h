#pragma once
#include <iostream>
#include <vector>
#include "mySQL_API_database.h"

/*
Класс, обеспечивающий базовые для дочерних классов взаимодействие с произвольной таблицей в mySQL.
Класс обеспечивает добавление/удаление пользователей, вход(с проверкой данных авторизации)/выход, чтение/изменение ряда столбцов.
Все табличные операции являются тривиальными, вся бизнес-логика реализована только в коде.
*/
class mySQL_API_table
{
public:
	mySQL_API_table(const std::string& tableName, mySQL_API_database* dbHandle, const std::vector<std::string>& collumnTypes, const std::vector<std::string>& collumnNames); // DDL-функционал
	
	bool checkIfColumnNamesOK(const std::vector<std::string>& collumnNames); // Проверка эквивалетности collumnNames (заданных в программе) и столбцов существующей таблицы
	
	// DML-функционал:
	void insert(const std::vector<std::string>& collumnNames, const std::vector<std::string>& values);	
	std::string selectOneRowWhereEqual(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue);	// SELECT fieldNameReturn FROM _tableName WHERE fieldNameCompare = 'compareValue' - только одна строка
	std::vector<std::string> selectSeveralRowsWhereEqual(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue);	// SELECT fieldNameReturn FROM _tableName WHERE fieldNameCompare = 'compareValue'
	std::vector<std::string> selectSeveralRowsWhereEqual2Filelds(const std::string& fieldNameReturn, const std::string& fieldName1Compare, const std::string& compareValue1, const std::string& fieldName2Compare, const std::string& compareValue2);	// SELECT fieldNameReturn FROM _tableName WHERE fieldName1Compare = 'compareValue1' AND fieldName2Compare ='compareValue2'
	std::vector<std::string> selectSeveralRowsWhereEqualAndIsNull(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue, const std::string& fieldNameIsNull);	// SELECT fieldNameReturn FROM _tableName WHERE fieldNameCompare ='compareValue' AND fieldNameIsNull IS NULL
	void updateValuesWhereEqual(const std::string& fieldNameUpdate, const std::string& newValue, const std::string& fieldNameCompare, const std::string& compareValue);	// UPDATE _tableName SET fieldNameUpdate ='newValue' WHERE fieldNameCompare ='compareValue'
	void updateValuesWhereEqualNoQuotes(const std::string& fieldNameUpdate, const std::string& newValue, const std::string& fieldNameCompare, const std::string& compareValue);	// UPDATE _tableName SET fieldNameUpdate = newValue WHERE fieldNameCompare ='compareValue' - нужно для констант вроде TIMESTAMP

private:
	std::string _tableName;
	mySQL_API_database* _dbHandle;
};


/* 
// Примеры использования:
constexpr const char* constTable_name = "test_table";
std::vector<std::string> collumnTypes = { "INT AUTO_INCREMENT PRIMARY KEY", "VARCHAR(255)" };
std::vector<std::string> collumnNames = { "id", "name" };

mySQL_API_database test_db(constDB_host, constDB_user, constDB_password);
test_db.connectAnyway(constDB_name);
mySQL_API_table tab1(constTable_name, &test_db, collumnTypes, collumnNames);

std::cout << "TEST: " << tab1.selectOneRowWhereEqual("name", "id", "1") << std::endl;
std::cout << "TEST: " << tab1.selectOneRowWhereEqual("id", "name", "Daria") << std::endl;

tab1.updateValuesWhereEqual("name", "nn1", "id", "4");
std::cout << "TEST: " << tab1.selectOneRowWhereEqual("id", "name", "Daria") << std::endl;

std::vector<std::string> retval = tab1.selectSeveralRowsWhereEqual("id", "name", "Daria");
for (auto it = retval.begin(); it != retval.end(); it++)
	std::cout << "[" << *it << "]";
std::cout << std::endl;
*/