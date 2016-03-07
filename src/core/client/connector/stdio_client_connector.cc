#include "core/client/connector/stdio_client_connector.h"

#include <iostream>

using namespace std;

bool StdioClientConnector::readExactly(void* buf, size_t size)
{
    cin.read(reinterpret_cast<char*>(buf), size);
    return cin.good();
}

bool StdioClientConnector::writeExactly(const void* buf, size_t size)
{
    cout.write(reinterpret_cast<const char*>(buf), size);
    return cout.good();
}

void StdioClientConnector::flush()
{
    cout.flush();
}
