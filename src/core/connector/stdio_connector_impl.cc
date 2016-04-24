#include "core/connector/stdio_connector_impl.h"

#include <iostream>

using namespace std;

StdioConnectorImpl::~StdioConnectorImpl()
{
}

bool StdioConnectorImpl::readExactly(void* buf, size_t size)
{
    cin.read(reinterpret_cast<char*>(buf), size);
    return cin.good();
}

bool StdioConnectorImpl::writeExactly(const void* buf, size_t size)
{
    cout.write(reinterpret_cast<const char*>(buf), size);
    return cout.good();
}

void StdioConnectorImpl::flush()
{
    cout.flush();
}
