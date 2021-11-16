#pragma once

#include "PipeStream/NamedPipeServerStream.hpp"
#include "MessageClient.hpp"

std::string CreatePipeName(std::string prefix);

MessageClient CreateClient(NamedPipeServerStream& pipe);
