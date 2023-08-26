#pragma once
#include "mySQL_API_table.h"
#include "mySQL_API_consts.h"

/*
Класс, обеспечивающий работу с таблицей сообщений.
Список полей (определены в mySQL_API_consts.h):
constMsg_id = "id"; - первичный ключ
constexpr const char* constMsg_sender = "sender_id";	- ссылка на идентификатор отправителя
constexpr const char* constMsg_receiver = "receiver_id";	- ссылка на идентификатор получателя
constexpr const char* constMsg_text = "text";	- текст сообщения
constexpr const char* constMsg_sent = "sent";	- время отправки
constexpr const char* constMsg_received = "received";	-время получения(==NULL, если не получено)
Класс обеспечивает добавление сообщений, поиск идентификаторов непрочитанных сообщений пользователя, установку статуса получения, чтение/изменение ряда столбцов.
Логика передачи сообщений:
A. при входе пользователя в систему:
	1) сервер ищет все недоставленные ему сообщения, 
	2) сообщения отправляются клиенту, 
	3) клиент при успешном получении уведомляет сервер,
	4) сервер получает подтверждение об успешной доставке, 
	5) сервер меняет время получения в базе с NULL на текущее;
B. при отправке сообщения:
	1) сервер сохраняет сообщение в базе,
	2) сервер проверяет онлайн-статус получателя,
	3) если получатель онлайн: A.2) - A.5)
*/
class mySQL_API_messagesTable : public mySQL_API_table
{
public:
	mySQL_API_messagesTable(mySQL_API_database* dbHandle);	// Вызывает конструктор родительского класса, который создаёт пустую таблицу при её отсутствии в БД

	void addMessage(const std::string& senderID, const std::string& receiverID, const std::string& text);	// Добавление нового сообщения в таблицу

	const std::string getSenderById(const std::string& messageId);
	const std::string getReceiverById(const std::string& messageId);
	const std::string getTextById(const std::string& messageId);
	const std::string getSentTimeById(const std::string& messageId);
	const std::string getReceivedTimeById(const std::string& messageId);

	std::vector<std::string> getUnreceivedMessagesIdsForUserId(const std::string& receiverId);	// Поиск идентификаторов сообщений, не доставленных пользователю

	void setReceived(const std::string& messageId);	// Изменение статуса доставки одного сообщения по id
	void setRecievedMultiple(std::vector<std::string> messagesIds);	// Изменение статуса доставки группы сообщений по vector ids

private:

};


/*
// Примеры использования:
mySQL_API_database test_db(constDB_host, constDB_user, constDB_password);
test_db.connectAnyway(constDB_name);

mySQL_API_messagesTable messages(&db1);

messages.addMessage("1", "2", "1-2");
messages.addMessage("2", "3", "2-3");
messages.addMessage("3", "1", "3-1");
messages.addMessage("1", "3", "1-3");
messages.addMessage("2", "1", "2-1");
messages.addMessage("3", "2", "3-2");

messages.setRecievedMultiple(messages.getUnreceivedMessagesIdsForUserId("1"));
*/