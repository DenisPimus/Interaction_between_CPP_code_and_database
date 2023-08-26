#include "../../include/mySQL_API/mySQL_API_messagesTable.h"

mySQL_API_messagesTable::mySQL_API_messagesTable(mySQL_API_database* dbHandle) :
	mySQL_API_table(constMessagesTable_name, dbHandle, constMessagesTable_collumnTypes, constMessagesTable_collumnNames)
{
}

void mySQL_API_messagesTable::addMessage(const std::string& senderID, const std::string& receiverID, const std::string& text)
{
	std::vector<std::string> collumnNames = { constMsg_sender, constMsg_receiver, constMsg_text };
	std::vector<std::string> values = { senderID, receiverID, text };
	insert(collumnNames, values);
#ifdef _DEBUG
	std::cout << "[DEBUG ] <mySQL_API_messagesTable::addMessage> Added message " << senderID << " -> " << receiverID << " to DB" << std::endl;
#endif 
}

const std::string mySQL_API_messagesTable::getSenderById(const std::string& messageId)
{
	return selectOneRowWhereEqual(constMsg_sender, constMsg_id, messageId);
}

const std::string mySQL_API_messagesTable::getReceiverById(const std::string& messageId)
{
	return selectOneRowWhereEqual(constMsg_receiver, constMsg_id, messageId);
}

const std::string mySQL_API_messagesTable::getTextById(const std::string& messageId)
{
	return selectOneRowWhereEqual(constMsg_text, constMsg_id, messageId);
}

const std::string mySQL_API_messagesTable::getSentTimeById(const std::string& messageId)
{
	return selectOneRowWhereEqual(constMsg_sent, constMsg_id, messageId);
}

const std::string mySQL_API_messagesTable::getReceivedTimeById(const std::string& messageId)
{
	return selectOneRowWhereEqual(constMsg_received, constMsg_id, messageId);
}

std::vector<std::string> mySQL_API_messagesTable::getUnreceivedMessagesIdsForUserId(const std::string& receiverId)
{
	return selectSeveralRowsWhereEqualAndIsNull(constMsg_id, constMsg_receiver, receiverId, constMsg_received);
}

void mySQL_API_messagesTable::setReceived(const std::string& messageId)
{
	updateValuesWhereEqualNoQuotes(constMsg_received, "CURRENT_TIMESTAMP", constMsg_id, messageId);
}

void mySQL_API_messagesTable::setRecievedMultiple(std::vector<std::string> messagesIds)
{
	for (auto it = messagesIds.begin(); it < messagesIds.end(); it++)
	{
		setReceived(*it);
	}
}