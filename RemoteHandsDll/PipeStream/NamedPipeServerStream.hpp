#pragma once

#include "PipeStream.hpp"
#include <string>

class NamedPipeServerStream : public PipeStream
{
public:
    static const int MaxAllowedServerInstances = -1;

    NamedPipeServerStream(const std::string& pipeName, PipeDirection direction, int maxNumberOfServerInstances,
                          PipeTransmissionMode transmissionMode);

    NamedPipeServerStream(const std::string& pipeName, PipeDirection direction, int maxNumberOfServerInstances,
                          PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize);

    ~NamedPipeServerStream() override;

private:
    void Create(const std::string& fullPipeName, PipeDirection direction, int maxNumberOfServerInstances,
                PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize);

public:
    void WaitForConnection();

    void Disconnect();

    bool IsConnected();

private:
    void CheckConnectOperationsServer() const;

    void CheckDisconnectOperations() const;
};
