#pragma once

#include <mutex>
#include <vector>
#include <cstdint>

class ClientSocket;
typedef ClientSocket* Client_PTR;

class CustomerTable
{
	static std::mutex my_mutex;
	static CustomerTable* instance;
	CustomerTable(uint16_t);

	uint16_t size;
	std::vector<Client_PTR> sockstat;
	

public:
	static CustomerTable& GetInstance(uint16_t);
	Client_PTR* operator[](uint16_t);
	bool isOn(uint16_t);
	bool isOFF(uint16_t);
};

