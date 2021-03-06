#include <string>
#include <filesystem>

namespace fs = std::filesystem;

#include "NamedPipeServerStream.hpp"
#include "Exceptions.hpp"

NamedPipeServerStream::NamedPipeServerStream(const std::string& pipeName, PipeDirection direction,
                                             int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode)
    : NamedPipeServerStream(pipeName, direction, maxNumberOfServerInstances, transmissionMode, 0, 0)
{
}

//---------------------------------------------------------------------------
NamedPipeServerStream::NamedPipeServerStream(const std::string& pipeName, PipeDirection direction,
                                             int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode,
                                             int inBufferSize, int outBufferSize)
    : PipeStream(direction, transmissionMode)
{
    const auto normalizedPipePath = (fs::path(R"(\\.\pipe)") / pipeName).string();

    Create(normalizedPipePath, direction, maxNumberOfServerInstances, transmissionMode, inBufferSize, outBufferSize);
}

//---------------------------------------------------------------------------
NamedPipeServerStream::~NamedPipeServerStream()
{
    state = PipeState::Closed;
}

//---------------------------------------------------------------------------
void
NamedPipeServerStream::Create(const std::string& fullPipeName, PipeDirection direction, int maxNumberOfServerInstances,
                              PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize)
{
    const auto pipeModes = static_cast<int>(transmissionMode) << 2 | static_cast<int>(transmissionMode) << 1;

    if (maxNumberOfServerInstances == MaxAllowedServerInstances)
    {
        maxNumberOfServerInstances = 255;
    }

    handle = SafePipeHandle(
        CreateNamedPipeA(fullPipeName.c_str(), static_cast<int>(direction), pipeModes, maxNumberOfServerInstances,
                         outBufferSize, inBufferSize, 0, nullptr), true);

    if (handle.IsInvalid())
    {
        throw IOException(GetLastError());
    }
}

//---------------------------------------------------------------------------
void NamedPipeServerStream::WaitForConnection()
{
    CheckConnectOperationsServer();

    if (!ConnectNamedPipe(handle.GetHandle(), nullptr))
    {
        const int errorCode = GetLastError();
        if (errorCode != ERROR_PIPE_CONNECTED)
        {
            throw IOException(errorCode);
        }

        if (errorCode == ERROR_PIPE_CONNECTED && state == PipeState::Connected)
        {
            throw InvalidOperationException();
        }
    }
    state = PipeState::Connected;
}

//---------------------------------------------------------------------------
void NamedPipeServerStream::Disconnect()
{
    CheckDisconnectOperations();

    if (!DisconnectNamedPipe(handle.GetHandle()))
    {
        throw IOException(GetLastError());
    }

    state = PipeState::Disconnected;
}

//---------------------------------------------------------------------------
bool NamedPipeServerStream::IsConnected()
{
    CheckDisconnectOperations();

    if (!PeekNamedPipe(handle.GetHandle(), nullptr, 0, nullptr, nullptr, nullptr))
    {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
void NamedPipeServerStream::CheckConnectOperationsServer() const
{
    if (state == PipeState::Closed || state == PipeState::Broken)
    {
        throw InvalidOperationException();
    }
}

//---------------------------------------------------------------------------
void NamedPipeServerStream::CheckDisconnectOperations() const
{
    if (state == PipeState::WaitingToConnect || state == PipeState::Disconnected || state == PipeState::Closed)
    {
        throw InvalidOperationException();
    }
}

//---------------------------------------------------------------------------
