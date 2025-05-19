#include "Logger.h"
#include "Exception.h"
#include "HeaderType.h"
#include "ClientHandler.h"
#include "CustomerTable.h"
#include "SystemExeption.h"
#include "SignalException.h"
#include "MessageReceiver.h"
#include "ProcessedMessage.h"
#include "DatabaseConection.h"


void ClientHandler::operator()(Client&& client)
{
	Logger::Info("Clientul a stabilit conexiunea! ");

	this->UserId = 0;
	this->mp = nullptr;
	this->receiver = nullptr;
	this->client = std::move(client);
	this->table = &(CustomerTable::GetInstance(_SIZE_));
	this->db = &(DatabaseConection::getInstance());

	while (true)
	{
		try {
			ProcessSignal();
			ClientDisconected();
			return;
		}
		catch (const int& e) {
			QuickResponse(static_cast<char>(Header::LOGOUT), static_cast<char>(Header::__OK__));
			Logger::Info("Utilizatorul:(" + std::to_string(UserId) + ") s-a deconectat!");
			RestartClient();
		}
	}
}

void ClientHandler::ProcessSignal()
{
	char rawData[1024];
	int bytesReceived;

	try {
		while (true) {

			bytesReceived = client.Receiver(rawData, sizeof(rawData));

			if (bytesReceived > 0) {
				HandlesMassege(rawData, bytesReceived);
			}
			else if (bytesReceived == 0 && UserId > 0) {
				NiceOUT();
			}
			else {
				forcExit();
			}
		}
	}
	catch (const Exception& e) {
		RestartClient();
		Logger::Info(e.what());
	}

}

void ClientHandler::HandlesMassege(char* rawData, int bytesReceived)
{
	try {
		this->receiver = new MessageReceiver(bytesReceived, rawData);
		ExecuteCommand(this->receiver->ParseHeader());
	}
	catch (const Exception& e) {
		Logger::Warning(e.what());
		WarningResponse("Date invalide!!!");
	}
}

void ClientHandler::NiceOUT()
{
	throw SignalException("Utilizatorul:(" + std::to_string(UserId) + ") s-a deconectat!");
}

void ClientHandler::forcExit()
{
	throw SignalException("Clientul a oprit conexiunea brusc!");
}

void ClientHandler::ExecuteCommand(PMS_PTR p)
{
	this->mp = p;
	Header Command = mp->getHeader();
	switch (Command) {
		case Header::LOGIN: this->ClientLogin(); break;
		case Header::SIGNUP: this->ClientSignUp(); break;
		case Header::TEXT: this->CustomerMessaging(); break;
		case Header::ADDCONT: this->AddContact(); break;
		case Header::LOGOUT: this->GetOut(); break;
		case Header::DELCONT: this->DeleteContact(); break;
		default: break;
	}
}

void ClientHandler::ClientLogin()
{
	int id = db->CkeckUser(mp->getPhoneNumber());
	if (id == -1) {
		WarningResponse("Utilizatorul nu exista !!!");
	}
	else if(db->CkeckPasswd(id, mp->getPassword()) == true)
	{
		std::string str = db->getUserName(id);
		ConfirmationResponse(str);
		Logger::Info("Utilizatoul cu numarul (" + std::to_string(id) + ") s-a autentificat!");
		this->UserId = id;
		Status_Turn(ON);
		try {
			this->LoadAgenda();
		}
		catch (const Exception& e) {
			WarningResponse("Nu ai agenda.");
			Logger::Warning(e.what());
		}
	}
	else {
		WarningResponse("Parola Incorecta!");
	}
}

void ClientHandler::ClientSignUp()
{
	int result = db->AddUser(mp->getUsername(), mp->getPhoneNumber(), mp->getPassword());
	Logger::Info(std::to_string(result));
	if (result > 0)
	{
		ConfirmationResponse("Inregistrare reusita!", static_cast<char>(Header::__OK__));
		int id = result;
		Logger::Info("Utilizatoul cu numarul (" + std::to_string(id) + ") s-a inregistrat!");
		this->UserId = id;
		Status_Turn(ON);
	}
	else {
		WarningResponse("Utilizator deja existent!!!");
	}
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void ClientHandler::CustomerMessaging()
{

	int contactId = db->GetContactIDByPhone(this->UserId, mp->getDest());

	if (contactId == -1) {
		Logger::Warning("Utilizatorul:(" + std::to_string(this->UserId) + ") a incercat sa trmitata mesaje inafara lista sale de contacte<-!");
	}
	else {
		if (this->table->isOn(contactId)) {
			RespondClient(contactId);
		}
		CreateTalkFile(contactId);
		Logger logger;
		logger.writeToFile(CatFileName(contactId), mp->getText(), mp->getSrc());
	}
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void ClientHandler::AddContact()
{
	std::string phone = mp->getPhoneNumber();
	int ContactId = db->CkeckUser(phone);

	int result = db->AddContact(this->UserId, ContactId, CatFileName(ContactId));

	switch (result) {
	case 0: 
		ConfirmationResponse(db->getUserName(ContactId), static_cast<char>(Header::ADDCONT));
		CreateTalkFile(ContactId);
		Logger::Info("Utilizatorul(" + std::to_string(this->UserId) + ") -> a adaugat la contacte pe Utilizator(" + std::to_string(ContactId) +").");
		break;
	case 1: WarningResponse("Contactul exista deja."); break;
	case -1: WarningResponse("Nu poti adauga propriul cont ca si contact.") ;break;
	case -2: WarningResponse("Userul respectiv nu exista in baza de date.") ;break;
	}
}

void ClientHandler::DeleteContact()
{
	std::string phone = mp->getPhoneNumber();
	int ContactId = db->CkeckUser(phone);

	int result = db->DeleteContact(this->UserId, ContactId);
	if (result == 0) {
		QuickResponse(static_cast<char>(Header::DELCONT), static_cast<char>(Header::__OK__));
		Logger::Info("Utilizatorul(" + std::to_string(this->UserId) + ") -> l-a sters de la contacte pe Utilizator(" + std::to_string(ContactId) + ").");
	}
	else
		QuickResponse(static_cast<char>(Header::DELCONT), static_cast<char>(Header::ERROR__));
}

void ClientHandler::GetOut()
{
	throw(404);
}

void ClientHandler::ConfirmationResponse(const std::string& str, char h)
{
	std::string reply;
	reply.push_back(h);
	reply += str;
	client.Sender(reply.c_str(), reply.size());
}

void ClientHandler::WarningResponse(const std::string& str)
{
	std::string reply;
	reply.push_back(static_cast<char>(Header::ERROR__));
	reply += str;
	client.Sender(reply.c_str(), reply.size());
}

void ClientHandler::QuickResponse(char stat, char flag)
{
	std::string reply;
	reply.push_back(stat);
	reply.push_back(flag);
	client.Sender(reply.c_str(), reply.size());
}

void ClientHandler::Status_Turn(bool stat)
{
	if (stat) {
		*((*table)[this->UserId]) = &(this->client);
	}
	else {
		*((*table)[this->UserId]) = nullptr;
	}
}

void ClientHandler::RespondClient(int contactId)
{
	std::string str;
	str.push_back(static_cast<char>(Header::TEXT));
	str += mp->getSrc();
	const std::string& reply = str + mp->getText();
	(*((*table)[contactId]))->Sender(reply.c_str(), reply.size());
}

void ClientHandler::RestartClient()
{
	this->UserId = 0;
	delete receiver;
	delete mp;
}

void ClientHandler::ClientDisconected()
{
	Status_Turn(OFF);
	Logger::Info("Conexiune_ Client Intrerupta! ");

	client.ClSocket();
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/
void ClientHandler::LoadAgenda() const
{
	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<std::string> tels;
	std::vector<std::string> files;

	db->GetUserContacts(UserId, ids, names, tels, files);

	if (names.empty() || tels.empty())
		throw SystemExeption("Server Cannot load agenda :(");

	std::string buffer;
	buffer.push_back(static_cast<char>(Header::AGENDA));

	for (int i = 0; i < ids.size(); i++)
	{
		buffer += tels[i];
		buffer += names[i];
		buffer.push_back(static_cast<char>(DISPEN));
	}

	client.Sender(buffer.c_str(), buffer.size());
	char raw[100];
	client.Receiver(raw, 100);

	if (files.empty())
		throw SystemExeption("Server Cannot load history massages :(");


	for (int i = 0; i < files.size(); i++)
	{
		buffer.clear();
		buffer.push_back(static_cast<char>(Header::TALKS));
		buffer += tels[i];
		Logger logger;
		buffer += logger.ReadFromFile(files[i]);
		client.Sender(buffer.c_str(), buffer.size());
		client.Receiver(raw, 100);
	}
	
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/


void ClientHandler::CreateTalkFile(int ContactId)
{
	std::string filename = CatFileName(ContactId);

	std::ofstream file(filename, std::ios::out | std::ios::app);
	file.close();
}

std::string ClientHandler::CatFileName(int ContactId)
{
	int prefix = MIN(this->UserId, ContactId);
	int sufix = MAX(this->UserId, ContactId);
	std::string filename = "C:\\Users\\BrsanChristian\\Desktop\\Arhitecturi\\" + std::to_string(prefix) + std::to_string(sufix) + ".txt";

	return filename;
}


