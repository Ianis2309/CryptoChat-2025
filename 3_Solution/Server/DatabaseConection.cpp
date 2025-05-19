//#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado60.tlb" rename("EOF", "ADOEOF")
//using namespace ADODB;
//#include <comdef.h>
#include <iostream>
#include "Logger.h"
#include "DatabaseConection.h"
#include "DatabaseException.h"

std::mutex DatabaseConection::my_mutex;
DatabaseConection* DatabaseConection::instance = nullptr;
DatabaseConection& DatabaseConection::getInstance()
{
    std::lock_guard<std::mutex> lock(my_mutex);

    if (instance == nullptr) {
        instance = new DatabaseConection();
    }
    return *instance;
}



DatabaseConection::DatabaseConection()
{
    //BC20CM\SQLEXPRESS (BC20cm\BrsanChristian)
    try {
        int n = CoInitialize(nullptr);
        conn.CreateInstance(__uuidof(Connection));
        conn->Open(
            "Provider=SQLOLEDB;"
            "Data Source=BC20CM\\SQLEXPRESS;"
            "Initial Catalog=ChatAppDB;"
            "Integrated Security=SSPI;",
            "", "", -1
        );

    }
    catch (_com_error& e) {
        std::wcerr << L" Eroare la conectare: " << e.ErrorMessage() << std::endl;

        if (e.Description().length() > 0)
            std::wcerr << L" Detalii SQL: " << (wchar_t*)e.Description() << std::endl;
        else
            std::wcerr << L" Nu exista detalii SQL disponibile." << std::endl;
    }

    if (conn->State != ADODB::adStateOpen)
        throw DatabaseException("Conection to DATABASE failed!");
}

DatabaseConection::~DatabaseConection()
{
    conn->Close();
    CoUninitialize();
}

////
////_CommandPtr DatabaseConection::initCmd()
////{
////    _CommandPtr cmd;
////    cmd.CreateInstance(__uuidof(Command));
////    cmd->ActiveConnection = conn;
////    cmd->CommandText = L"AddUser";
////    cmd->CommandType = adCmdStoredProc;
////    return cmd;
////}


std::wstring DatabaseConection::GetName()
{
    _RecordsetPtr sel;
    sel.CreateInstance(__uuidof(Recordset));
    sel->Open(
        L"SELECT DB_NAME() AS DatabaseName;", 
        conn.GetInterfacePtr(),
        adOpenStatic,
        adLockReadOnly,
        adCmdText
    );

    std::wstring dbName = L"Salut";
    if (!sel->ADOEOF) {
        _variant_t val = sel->Fields->GetItem(L"DatabaseName")->Value;
        if (val.vt != VT_NULL) {
            dbName = (wchar_t*)_bstr_t(val);
        }
    }

    sel->Close();
    return dbName;
}

std::string DatabaseConection::_GetName_str()
{
    std::wstring v = GetName();
    std::string s(v.begin(), v.end());
    return s;
}

std::string DatabaseConection::getUserName(long UserId)
{

    std::wstring query = L"SELECT Username FROM Users WHERE UserID = " + std::to_wstring(UserId) + L";";
    _RecordsetPtr sel;
    sel.CreateInstance(__uuidof(Recordset));
    sel->Open(
        _bstr_t(query.c_str()),
        conn.GetInterfacePtr(),
        adOpenStatic,
        adLockReadOnly,
        adCmdText
    );

    if (!sel->ADOEOF) {
        _variant_t val = sel->Fields->GetItem(L"Username")->Value;
        if (val.vt != VT_NULL) {
            std::wstring bstr = (wchar_t*)_bstr_t(val);
            std::string str(bstr.begin(), bstr.end());
            return str;
        }
    }

    sel->Close();
    return "";
}

int DatabaseConection::AddUser(std::string _username, std::string _phone, std::string _password)
{
    _CommandPtr cmd;
    cmd.CreateInstance(__uuidof(Command));
    cmd->ActiveConnection = conn;
    cmd->CommandText = L"AddUser";
    cmd->CommandType = adCmdStoredProc;

    _bstr_t username = string_to_wstring(_username).c_str();
    _bstr_t password = string_to_wstring(_password).c_str();
    _bstr_t phone = string_to_wstring(_phone).c_str();

    _ParameterPtr paramUsername, paramPhone, paramPasswd, paramReturn;

    // Parametru RETURN VALUE
    paramReturn = cmd->CreateParameter(L"@ReturnValue", adInteger, adParamReturnValue, sizeof(int));
    cmd->Parameters->Append(paramReturn);

    // Parametru IN: @UserName
    paramUsername = cmd->CreateParameter("@UserName", adVarWChar, adParamInput, 50, username);
    cmd->Parameters->Append(paramUsername);

    // Parametru IN: @PasswordHash
    paramPasswd = cmd->CreateParameter("@PasswordHash", adVarWChar, adParamInput, 256, password);
    cmd->Parameters->Append(paramPasswd);

    // Parametru IN: @PhoneNumber
    paramPhone = cmd->CreateParameter("@PhoneNumber", adVarChar, adParamInput, 15, phone);
    cmd->Parameters->Append(paramPhone);

    // Executa Procedura Stocata
    cmd->Execute(nullptr, nullptr, adCmdStoredProc);

    int returnCode = paramReturn->Value;

    return returnCode;
}

long DatabaseConection::CkeckUser(std::string phone)
{
    try{
    std::wstring _phone(phone.begin(), phone.end());
    std::wstring query = L"SELECT UserID FROM Users WHERE PhoneNumber = '" + _phone + L"'";
    _RecordsetPtr sel;
    sel.CreateInstance(__uuidof(Recordset));
    if (conn->State != adStateOpen) {
        std::wcerr << L"[Eroare] Conexiunea nu este deschisă!" << std::endl;
        return -1;
    }
    sel->Open(
        _bstr_t(query.c_str()),
        conn.GetInterfacePtr(),
        adOpenStatic,
        adLockReadOnly,
        adCmdText
    );
    if (!sel->ADOEOF) {
        _variant_t val = sel->Fields->GetItem(L"UserID")->Value;
        if (val.vt != VT_NULL) {
            return val.intVal;
        }
    }

    sel->Close();
    }
    catch (_com_error& e) {
        std::wcerr << L"Eroare ADO: " << e.Description() << std::endl;
        std::wcerr << L"Detalii: " << e.ErrorMessage() << std::endl;
    }

    return -1;
}

bool DatabaseConection::CkeckPasswd(long id, std::string supposedpass)
{
    std::wstring userID = std::to_wstring(id);
    std::wstring query = L"SELECT PasswordHash FROM Users WHERE UserID = " + userID;
    _RecordsetPtr sel;
    sel.CreateInstance(__uuidof(Recordset));
    sel->Open(
        _bstr_t(query.c_str()),
        conn.GetInterfacePtr(),
        adOpenStatic,
        adLockReadOnly,
        adCmdText
    );

    std::string temp = "";
    if (!sel->ADOEOF) {
        _variant_t val = sel->Fields->GetItem(L"PasswordHash")->Value;
        if (val.vt != VT_NULL) {
            std::wstring surepass = (wchar_t*)_bstr_t(val); 
            temp = wstring_to_string(surepass); 
        }
    }

    sel->Close();

    return (strcmp(temp.c_str(), supposedpass.c_str()) == 0);
}

int DatabaseConection::AddContact(long userId, long contactId, const std::string& _filename)
{
    _CommandPtr cmd;
    cmd.CreateInstance(__uuidof(Command));
    cmd->ActiveConnection = conn;
    cmd->CommandText = L"AddContact";
    cmd->CommandType = adCmdStoredProc;

    _bstr_t filename = string_to_wstring(_filename).c_str();

    // Parametru de input: @UserID
    _ParameterPtr pUser = cmd->CreateParameter(L"@UserID", adInteger, adParamInput, sizeof(int), userId);
    cmd->Parameters->Append(pUser);

    // Parametru de input: @ContactUserID
    _ParameterPtr pContact = cmd->CreateParameter(L"@ContactUserID", adInteger, adParamInput, sizeof(int), contactId);
    cmd->Parameters->Append(pContact);

    // Parametru de output: @ReturnCode
    _ParameterPtr pReturnCode = cmd->CreateParameter(L"@ReturnCode", adInteger, adParamOutput, sizeof(int));
    cmd->Parameters->Append(pReturnCode);

    // Parametru de input: @ConversationFile
    _ParameterPtr pFileConversation = cmd->CreateParameter(L"@ConversationFile", adVarWChar, adParamInput, 255, filename);
    cmd->Parameters->Append(pFileConversation);

    cmd->Execute(nullptr, nullptr, adCmdStoredProc);

    int returnCode = pReturnCode->Value.intVal;

    return returnCode;
}

void DatabaseConection
     ::GetUserContacts(int id, 
                       std::vector<int>& ids, 
                       std::vector<std::string>& name, 
                       std::vector<std::string>& tel, 
                       std::vector<std::string>& files
                       )
{
    _CommandPtr cmd;
    cmd.CreateInstance(__uuidof(Command));
    cmd->ActiveConnection = conn;
    cmd->CommandType = adCmdStoredProc;
    cmd->CommandText = L"ShowContacts";

    _ParameterPtr paramUserId = cmd->CreateParameter(
        L"@UserID", adInteger, adParamInput, sizeof(int), id
    );
    cmd->Parameters->Append(paramUserId);

    _RecordsetPtr rs;
    rs = cmd->Execute(nullptr, nullptr, adCmdStoredProc);

    while (!rs->ADOEOF) {
        //ContactInfo contact;
        _variant_t val;

        val = rs->Fields->GetItem(L"UserID")->Value;
        int userId = (val.vt == VT_NULL) ? -1 : val.intVal;
        ids.push_back(userId);

        val = rs->Fields->GetItem(L"Username")->Value;
        std::wstring un = (val.vt == VT_NULL) ? L"" : (wchar_t*)_bstr_t(val);
        std::string username(un.begin(), un.end());
        name.push_back(username);

        val = rs->Fields->GetItem(L"PhoneNumber")->Value;
        std::wstring phone = (val.vt == VT_NULL) ? L"" : (wchar_t*)_bstr_t(val);
        std::string phoneNumber(phone.begin(), phone.end());
        tel.push_back(phoneNumber);

        val = rs->Fields->GetItem(L"ConversationFile")->Value;
        std::wstring file = (val.vt == VT_NULL) ? L"" : (wchar_t*)_bstr_t(val);
        std::string filename(file.begin(), file.end());
        files.push_back(filename);

        rs->MoveNext();
    }

    rs->Close();
}

int DatabaseConection::DeleteContact(int userId, int contactId)
{
    _CommandPtr cmd;
    cmd.CreateInstance(__uuidof(Command));
    cmd->ActiveConnection = conn;
    cmd->CommandText = L"DeleteContact";
    cmd->CommandType = adCmdStoredProc;

    // Parametru de RETURN
    _ParameterPtr returnParam = cmd->CreateParameter(
        L"@RETURN_VALUE", adInteger, adParamReturnValue, sizeof(int));
    cmd->Parameters->Append(returnParam);

    // Parametru IN: @UserID
    _ParameterPtr paramUser = cmd->CreateParameter(
        L"@UserID", adInteger, adParamInput, sizeof(int), userId);
    cmd->Parameters->Append(paramUser);

    // Parametru IN: @ContactUserID
    _ParameterPtr paramContact = cmd->CreateParameter(
        L"@ContactUserID", adInteger, adParamInput, sizeof(int), contactId);
    cmd->Parameters->Append(paramContact);

    // Execută procedura
    cmd->Execute(nullptr, nullptr, adCmdStoredProc);

    // Returnează valoarea din RETURN
    return returnParam->Value.intVal;
}


long DatabaseConection::GetContactIDByPhone(long UserId, std::string _Phone)
{
    _CommandPtr cmd;
    cmd.CreateInstance(__uuidof(Command));
    cmd->ActiveConnection = conn;
    cmd->CommandText = L"GetContactIDByPhone";
    cmd->CommandType = adCmdStoredProc;

    _bstr_t Phone = _Phone.c_str();

    // Parametru @UserID (input)
    _ParameterPtr pUserID = cmd->CreateParameter(L"@UserID", adInteger, adParamInput, sizeof(int), UserId);
    cmd->Parameters->Append(pUserID);

    // Parametru @PhoneNumber (input)
    _ParameterPtr pPhone = cmd->CreateParameter(L"@PhoneNumber", adVarChar, adParamInput, 20, Phone);
    cmd->Parameters->Append(pPhone);

    // Parametru @ContactID (output)
    _ParameterPtr pOutContact = cmd->CreateParameter(L"@ContactID", adInteger, adParamOutput, sizeof(int));
    cmd->Parameters->Append(pOutContact);

    cmd->Execute(nullptr, nullptr, adCmdStoredProc);

    int contactID = pOutContact->Value.intVal;

    return contactID;
}



std::string DatabaseConection::wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        Logger::Warning("Eroare la conversia wstring in string");
        return "";
    }
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

std::wstring DatabaseConection::string_to_wstring(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size == 0) {
        Logger::Warning("Eroare la conversia string în wstring");
        return L"";
    }
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}