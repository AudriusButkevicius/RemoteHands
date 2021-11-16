#pragma once

#include <string>
#include <map>
#include <utility>
#include <vector>
#include "MessageClient.hpp"
#include "Client.hpp"
#include "RemoteFunctions.hpp"
#include <iostream>
#include "PipeStream/NamedPipeServerStream.hpp"
#include "PipeStream/Exceptions.hpp"
#include <atomic>
#include <boost/stacktrace.hpp>


std::atomic<MessageClient*> messageClient;
std::mutex callMutex;
std::map<std::string, RemoteFunction*> functions = {};

void ffi_call_handler(ffi_cif* cif, void* ret, void* args[], void* in)
{
    auto* func = static_cast<RemoteFunction*>(in);
    // Store zero by default
    *reinterpret_cast<uintptr_t*>(ret) = 0;
    try
    {
        // Single threaded, because we need to wait for responses etc.
        // Probably want to make the protocol async.
        std::lock_guard g(callMutex);
        auto client = messageClient.load();
        if (client == nullptr)
        {
            return;
        }

        std::vector<uintptr_t> argPtrVec((uintptr_t*)args, (uintptr_t*)args + cif->nargs);
        std::vector<uintptr_t> argValue;
        argValue.reserve(argPtrVec.size());
        for (const auto& item : argPtrVec)
        {
            argValue.push_back(*reinterpret_cast<uintptr_t*>(item));
        }
        client->Send(RemoteCallRequest(func->Name(), std::move(argValue)));

        auto msg = client->Receive();
        if (msg == nullptr)
        {
            return;
        }
        auto response = static_cast<RemoteCallResponse*>(msg.get());
        *reinterpret_cast<uintptr_t*>(ret) = response->GetReturnValue();
        if (response->GetFlags() == 1)
        {
            std::cout << boost::stacktrace::stacktrace();
        }
    }
    catch (...)
    {
        messageClient.store(nullptr);
        std::cout << "Exception in ffi call handler for " << func->Name() << std::endl;
    }
}

RemoteFunction::RemoteFunction(std::string _name, uint32_t _numArgs) :
    name(std::move(_name)),
    args(new ffi_type*[_numArgs]),
    cif(),
    address(0)
{
    for (uint32_t i = 0; i < _numArgs; i++)
    {
        args[i] = &ffi_type_pointer;
    }

    auto status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, _numArgs, &ffi_type_pointer, args);
    if (status != FFI_OK)
    {
        std::stringstream ss;
        ss << "FFI prepare failed: " << status << std::endl;
        throw std::exception(ss.str().c_str());
    }

    closure = static_cast<ffi_closure*>(ffi_closure_alloc(sizeof(ffi_closure), reinterpret_cast<void**>(&address)));

    status = ffi_prep_closure_loc(closure, &cif, ffi_call_handler,
                                  this, &address);
    if (status != FFI_OK)
    {
        std::stringstream ss;
        ss << "FFI closure prepare failed: " << status << std::endl;
        throw std::exception(ss.str().c_str());
    }
}

RemoteFunction::~RemoteFunction()
{
    delete[] args;
    ffi_closure_free(closure);
}


const std::string& RemoteFunction::Name() const
{
    return name;
}

uintptr_t RemoteFunction::Address() const
{
    return address;
}


RemoteFunction* GetRemoteFunction(const std::string& name)
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        return it->second;
    }
    return nullptr;
}


uintptr_t AllocateRemoteFunction(const std::string& name, uint32_t numArgs)
{
    auto pFunction = GetRemoteFunction(name);
    if (pFunction != nullptr)
    {
        return pFunction->Address();
    }
    std::cout << "Creating remote function " << name << " with " << numArgs << " args" << std::endl;
    pFunction = new RemoteFunction(name, numArgs);
    functions.insert_or_assign(name, pFunction);
    return pFunction->Address();
}

bool FreeRemoteFunction(const std::string& name)
{
    auto func = GetRemoteFunction(name);
    if (func == nullptr)
    {
        return false;
    }
    functions.erase(name);
    delete func;
    return true;
}

[[noreturn]] void RemoteFunctionDataThread(void*)
{
    const auto name = CreatePipeName("mbamd-");
    while (true)
    {
        try
        {
            NamedPipeServerStream pipe(name, PipeDirection::InOut, 1, PipeTransmissionMode::Message);
            pipe.WaitForConnection();
            std::cout << "Got a connection on function data thread" << std::endl;
            auto client = CreateClient(pipe);
            messageClient.store(&client);
            try
            {
                while (pipe.IsConnected())
                {
                    std::this_thread::yield();
                }
            }
            catch (...)
            {
            }
            messageClient.store(nullptr);
            std::cout << "Client on function data thread went away" << std::endl;
            pipe.Disconnect();
        }
        catch (InvalidOperationException*)
        {
        }
        catch (IOException*)
        {
        }
        catch (...)
        {
        }
    }
}
