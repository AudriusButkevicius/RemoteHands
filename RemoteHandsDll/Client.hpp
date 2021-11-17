#pragma once

#include "MessageClient.hpp"
#include "PipeStream/NamedPipeServerStream.hpp"

std::string CreatePipeName(std::string prefix);

MessageClient CreateClient(NamedPipeServerStream& pipe);
