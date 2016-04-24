#ifndef CORE_CONNECTOR_STDIO_CONNECTOR_IMPL_H_
#define CORE_CONNECTOR_STDIO_CONNECTOR_IMPL_H_

#include <cstddef>

#include "core/connector/connector_impl.h"

class StdioConnectorImpl : public ConnectorImpl {
public:
    ~StdioConnectorImpl() override;

    bool readExactly(void* buf, size_t size) override;
    bool writeExactly(const void* buf, size_t size) override;
    void flush() override;
};

#endif // CORE_CONNECTOR_STDIO_CONNECTOR_IMPL_H_
