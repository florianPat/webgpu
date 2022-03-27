#pragma once

#include "Types.h"

struct INetworkAbstraction
{
    INetworkAbstraction() = default;
    INetworkAbstraction(const INetworkAbstraction& other) = delete;
    INetworkAbstraction& operator=(const INetworkAbstraction& rhs) = delete;
    INetworkAbstraction(INetworkAbstraction&& other) = default;
    INetworkAbstraction& operator=(INetworkAbstraction&& rhs) = default;
    virtual ~INetworkAbstraction() {}

    /*
    struct NetworkBuffer
    {
        uint8_t* bytes;
        uint32_t size;
    };

    void send(uint8_t* buffer, bool reliable);
    NetworkBuffer receive();
    void disconnectFromAllEndpoints();
    int32_t getConnectFlag();
    void destruct();
     */
};