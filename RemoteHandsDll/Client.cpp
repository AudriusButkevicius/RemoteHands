#pragma once

#include <filesystem>
#include "PipeStream/NamedPipeServerStream.hpp"
#include "MessageClient.hpp"

namespace fs = std::filesystem;

std::string CreatePipeName(std::string prefix)
{
    fs::path name(prefix);

    auto pid = GetCurrentProcessId();
    name += fs::path(std::to_string(pid)).filename();

    return name.string();
}

//---------------------------------------------------------------------------
MessageClient CreateClient(NamedPipeServerStream& pipe)
{
    MessageClient client(pipe);

    client.RegisterMessage<StatusResponse>();
    client.RegisterMessage<StatusWithValueResponse>();
    client.RegisterMessage<CloseProcessRequest>();
    client.RegisterMessage<IsValidRequest>();
    client.RegisterMessage<ReadMemoryRequest>();
    client.RegisterMessage<WriteMemoryRequest>();
    client.RegisterMessage<AllocateMemoryRequest>();
    client.RegisterMessage<FreeMemoryRequest>();
    client.RegisterMessage<QueryMemoryRequest>();
    client.RegisterMessage<ProtectMemoryRequest>();
    client.RegisterMessage<CallFunctionRequest>();
    client.RegisterMessage<AllocateConsoleRequest>();
    client.RegisterMessage<CreateFunctionRequest>();
    client.RegisterMessage<FreeFunctionRequest>();
    client.RegisterMessage<RemoteCallRequest>();
    client.RegisterMessage<RemoteCallResponse>();

    return client;
}
