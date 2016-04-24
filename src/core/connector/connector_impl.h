#ifndef CORE_CONNECTOR_CONNECTOR_IMPL_H_
#define CORE_CONNECTOR_CONNECTOR_IMPL_H_

#include <cstddef>

class ConnectorImpl {
public:
    virtual ~ConnectorImpl() {}

    virtual bool readExactly(void* buf, size_t size) = 0;
    virtual bool writeExactly(const void* buf, size_t size) = 0;
    virtual void flush() = 0;
};

#endif // CORE_CONNECTOR_CONNECTOR_IMPL_H_
