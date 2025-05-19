#pragma once
#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado60.tlb" rename("EOF", "ADOEOF")
#include <comdef.h>
#include <string>
#include <vector>
#include <mutex>
using namespace ADODB;


class DatabaseConection
{
private:
	static DatabaseConection* instance;
	static std::mutex my_mutex;
	DatabaseConection();

	_ConnectionPtr	 conn;
	//_RecordsetPtr	 sel;
	//_CommandPtr	 cmd;

public:
	DatabaseConection(const DatabaseConection&) = delete;
	void operator=(const DatabaseConection&) = delete;
	static DatabaseConection& getInstance();
	~DatabaseConection();

	std::wstring GetName();
	std::string _GetName_str();

	std::string getUserName(long);
	int AddUser(std::string, std::string, std::string);
	long CkeckUser(std::string);
	bool CkeckPasswd(long, std::string);
	int AddContact(long, long, const std::string&);
	int DeleteContact(int, int);
	void GetUserContacts(int, std::vector<int>&, std::vector<std::string>&, std::vector<std::string>&, std::vector<std::string>&);
	long GetContactIDByPhone(long, std::string);

	std::string wstring_to_string(const std::wstring& wstr);
	std::wstring string_to_wstring(const std::string& str);
};