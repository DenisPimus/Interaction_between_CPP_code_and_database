#include "../../include/mySQL_API/mySQL_API_table.h"

mySQL_API_table::mySQL_API_table(const std::string& tableName, mySQL_API_database* dbHandle, const std::vector<std::string>& collumnTypes, const std::vector<std::string>& collumnNames) :
	_tableName(tableName), _dbHandle(dbHandle)//, _collumnTypes(collumnTypes), _collumnNames(collumnNames)
{
	std::string req;
	if (!checkIfColumnNamesOK(collumnNames))
	{
		// Если таблица уже существует, но отличается структурой, нужно прервать работу!
		std::cout << "[FAILED] <mySQL_API_table::mySQL_API_table> Could not connect to " << tableName << " in " << _dbHandle->getName() << "!" << std::endl;
		// TODO: throw exception
		return;
	}
	else
	{
		// Либо таблицы ещё нет, тогда создаём, либо она есть и правильная (на этот случай, в запросе стоит IF NOT EXISTS).
		req = "CREATE TABLE IF NOT EXISTS " + _tableName + "(";
		for (size_t i = 0; i < collumnTypes.size(); i++)
		{
			if (i != 0)
				req = req + ", ";
			req = req + collumnNames.at(i) + " " + collumnTypes.at(i);
		}
		req = req + ")";
#ifdef _DEBUG
		std::cout << "[DEBUG ] <mySQL_API_table::mySQL_API_table> CREATE request: " << req << std::endl;
#endif 
		mysql_query(_dbHandle->getHandle(), req.c_str());
	}
}

bool mySQL_API_table::checkIfColumnNamesOK(const std::vector<std::string>& collumnNames)
{
	std::string tmpStr;
	std::vector<std::string> tmpVect;
	bool tableExists = false;
	MYSQL_RES* res;
	MYSQL_ROW row;
	std::string req = "SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = '" + _dbHandle->getName() + "' AND TABLE_NAME = '" + _tableName + "'";
	mysql_query(_dbHandle->getHandle(), req.c_str());
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::checkIfColumnNamesOK> SELECT request: " << req << std::endl;
#endif 
	if (res = mysql_store_result(_dbHandle->getHandle())) {
		while (row = mysql_fetch_row(res))
		{
			tableExists = true;
			if (mysql_num_fields(res) == 0)
			{
#ifdef _DEBUG
				std::cout << "[FAILED] <mySQL_API_table::checkIfColumnNamesOK> SELECT returned NULL! " << std::endl;
#endif 
				// Таблица существует, но столбцов в ней нет - нужно добавлять
				// TODO: Цикл ALTER TABLE ... ADD COLUMN ...
				return false;
			}
			tmpStr = row[0];
			tmpVect.push_back(tmpStr);
		}
		if (tableExists)
		{
			if (tmpVect.size() != collumnNames.size())	// Таблица уже существует, но с дручим числом столбцов
			{
#ifdef _DEBUG
				std::cout << "[FAILED] <mySQL_API_table::checkIfColumnNamesOK> Table exists and contains wrong number of columns! Manual redesign needed!" << std::endl;
#endif
				return false;
			}
			else
			{
				for (size_t i = 0; i < tmpVect.size(); ++i)	// TODO: Может быть другой порядок столбцов - сравнивать не по-порядку, а с помощью set.
					if (tmpVect.at(i).compare(collumnNames.at(i)) != 0)	// Таблица уже существует, но с другими именами столбцов
					{
#ifdef _DEBUG
						std::cout << "[FAILED] <mySQL_API_table::checkIfColumnNamesOK> Table exists and contains different columns! Manual redesign needed!" << std::endl;
#endif
						return false;
					}
				// TODO: Проверка типов?
			}
		}
	}
	else
	{
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_table::checkIfColumnNamesOK> " << mysql_error(_dbHandle->getHandle()) << std::endl;
#endif
		return false;
	}
	return true;
}

void mySQL_API_table::insert(const std::vector<std::string>& collumnNames, const std::vector<std::string>& values)
{
	std::string req = "INSERT " + _tableName + "(";
	for (size_t i = 0; i < collumnNames.size(); i++)
	{
		if (i != 0)
			req = req + ", ";
		req = req + collumnNames.at(i) + " ";
	}
	req = req + ") VALUES (";
	for (size_t i = 0; i < values.size(); i++)
	{
		if (i != 0)
			req = req + ", ";
		req = req + "'" + values.at(i) + "' ";
	}
	req = req + ")";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::insert> INSERT request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());
}

std::string mySQL_API_table::selectOneRowWhereEqual(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue)
{
	std::vector<std::string> tmpVect = selectSeveralRowsWhereEqual(fieldNameReturn, fieldNameCompare, compareValue);
	std::string retVal;

	if (tmpVect.size() == 0)
	{
#ifdef _DEBUG
		std::cout << "[ WARN ] <mySQL_API_table::selectOneRowWhereEqual> SELECT returned NULL! " << std::endl;
#endif 
		return std::string();
	}
	if (tmpVect.size() > 1)
	{
#ifdef _DEBUG
		std::cout << "[ WARN ] <mySQL_API_table::selectOneRowWhereEqual> SELECT returned several rows! Using the first!" << std::endl;
#endif 
	}
	retVal = tmpVect[0];
	return retVal;
}

std::vector<std::string> mySQL_API_table::selectSeveralRowsWhereEqual(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue)
{
	std::vector<std::string> retVal;
	std::string tmpStr;
	MYSQL_RES* res;
	MYSQL_ROW row;
	std::string req = "SELECT " + fieldNameReturn + " FROM " + _tableName + " WHERE " + fieldNameCompare + "='" + compareValue + "'";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::selectSeveralRowsWhereEqual> SELECT request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());

	if (res = mysql_store_result(_dbHandle->getHandle())) {
		while (row = mysql_fetch_row(res)) 
		{
			if (mysql_num_fields(res) == 0)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqual> SELECT returned NULL! " << std::endl;
#endif 
			}
			if (mysql_num_fields(res) > 1)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqual> SELECT returned several collumns! " << std::endl;
#endif 
			}
			tmpStr = row[0];
			retVal.push_back(tmpStr);
		}
	}
	else
	{
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_table::selectSeveralRowsWhereEqual> " << mysql_error(_dbHandle->getHandle()) << std::endl;
#endif
		return std::vector<std::string>();
	}

	return retVal;
}

std::vector<std::string> mySQL_API_table::selectSeveralRowsWhereEqual2Filelds(const std::string& fieldNameReturn, const std::string& fieldName1Compare, const std::string& compareValue1, const std::string& fieldName2Compare, const std::string& compareValue2)
{
	std::vector<std::string> retVal;
	std::string tmpStr;
	MYSQL_RES* res;
	MYSQL_ROW row;
	std::string req = "SELECT " + fieldNameReturn + " FROM " + _tableName + " WHERE " + fieldName1Compare + "='" + compareValue1 + "' AND " + fieldName2Compare + "='" + compareValue2 + "'";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::selectSeveralRowsWhereEqual2Filelds> SELECT request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());

	if (res = mysql_store_result(_dbHandle->getHandle())) {
		while (row = mysql_fetch_row(res))
		{
			if (mysql_num_fields(res) == 0)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqual2Filelds> SELECT returned NULL! " << std::endl;
#endif 
			}
			if (mysql_num_fields(res) > 1)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqual2Filelds> SELECT returned several collumns! " << std::endl;
#endif 
			}
			tmpStr = row[0];
			retVal.push_back(tmpStr);
		}
	}
	else
	{
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_table::selectSeveralRowsWhereEqual2Filelds> " << mysql_error(_dbHandle->getHandle()) << std::endl;
#endif
		return std::vector<std::string>();
	}

	return retVal;
}

std::vector<std::string> mySQL_API_table::selectSeveralRowsWhereEqualAndIsNull(const std::string& fieldNameReturn, const std::string& fieldNameCompare, const std::string& compareValue, const std::string& fieldNameIsNull)
{
	std::vector<std::string> retVal;
	std::string tmpStr;
	MYSQL_RES* res;
	MYSQL_ROW row;
	std::string req = "SELECT " + fieldNameReturn + " FROM " + _tableName + " WHERE " + fieldNameCompare + "='" + compareValue + "' AND " + fieldNameIsNull + " IS NULL";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::selectSeveralRowsWhereEqualAndIsNull> SELECT request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());

	if (res = mysql_store_result(_dbHandle->getHandle())) {
		while (row = mysql_fetch_row(res))
		{
			if (mysql_num_fields(res) == 0)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqualAndIsNull> SELECT returned NULL! " << std::endl;
#endif 
			}
			if (mysql_num_fields(res) > 1)
			{
#ifdef _DEBUG
				std::cout << "[ WARN ] <mySQL_API_table::selectSeveralRowsWhereEqualAndIsNull> SELECT returned several collumns! " << std::endl;
#endif 
			}
			tmpStr = row[0];
			retVal.push_back(tmpStr);
		}
	}
	else
	{
#ifdef _DEBUG
		std::cout << "[FAILED] <mySQL_API_table::selectSeveralRowsWhereEqualAndIsNull> " << mysql_error(_dbHandle->getHandle()) << std::endl;
#endif
		return std::vector<std::string>();
	}

	return retVal;
}

void mySQL_API_table::updateValuesWhereEqual(const std::string& fieldNameUpdate, const std::string& newValue, const std::string& fieldNameCompare, const std::string& compareValue)
{
	std::string req = "UPDATE " + _tableName + " SET " + fieldNameUpdate + " ='" + newValue + "' WHERE " + fieldNameCompare + "='" + compareValue + "'";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::updateValuesWhereEqual> UPDATE request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());
}

void mySQL_API_table::updateValuesWhereEqualNoQuotes(const std::string& fieldNameUpdate, const std::string& newValue, const std::string& fieldNameCompare, const std::string& compareValue)
{
	std::string req = "UPDATE " + _tableName + " SET " + fieldNameUpdate + " = " + newValue + " WHERE " + fieldNameCompare + "='" + compareValue + "'";
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_table::updateValuesWhereEqualNoQuotes> UPDATE request: " << req << std::endl;
#endif 
	mysql_query(_dbHandle->getHandle(), req.c_str());
}
