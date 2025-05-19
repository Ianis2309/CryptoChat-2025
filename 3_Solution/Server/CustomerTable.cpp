#include "CustomerTable.h"
#include "ClientSocket.h"

#include <iostream>

std::mutex CustomerTable::my_mutex;
CustomerTable* CustomerTable::instance = nullptr;
CustomerTable& CustomerTable::GetInstance(uint16_t size)
{
    std::lock_guard<std::mutex> lock(my_mutex);

    if (instance == nullptr) {
        instance = new CustomerTable(size);
    }
    return *instance;
}

Client_PTR* CustomerTable::operator[](uint16_t index)
{
    return &(sockstat[index]);
}

bool CustomerTable::isOn(uint16_t index)
{
    return (sockstat[index] != nullptr);
}

bool CustomerTable::isOFF(uint16_t index)
{
    return (sockstat[index] == nullptr);
}

CustomerTable::CustomerTable(uint16_t size)
{
    this->size = size;
    sockstat.resize(size);
    for (int i = 0; i < size; i++) {
        sockstat[i] = nullptr;
    }
}