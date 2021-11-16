#pragma once

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <ffi.h>
#include "MessageClient.hpp"
#include <iostream>
#include <sstream>
#include <deque>
#include <mutex>

class RemoteFunction
{
public:
    RemoteFunction(std::string _name, unsigned int _numArgs);

    ~RemoteFunction();

    [[nodiscard]] const std::string& Name() const;

    [[nodiscard]] uintptr_t Address() const;

private:
    std::string name;
    ffi_type** args;

    ffi_cif cif;
    ffi_closure* closure;

    uintptr_t address;
};

RemoteFunction* GetRemoteFunction(const std::string& name);

uintptr_t AllocateRemoteFunction(const std::string& name, unsigned int numArgs);

bool FreeRemoteFunction(const std::string& name);

[[noreturn]] void RemoteFunctionDataThread(void*);
