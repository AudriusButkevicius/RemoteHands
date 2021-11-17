#include "Messages.hpp"

#include <csignal>
#include <ffi.h>
#include <iostream>
#include <stdexcept>

#include "MemoryHelper.hpp"
#include "MessageClient.hpp"
#include "RemoteFunctions.hpp"

void SignalHandler(int signal)
{
    throw std::exception("Access Violation");
}

//---------------------------------------------------------------------------
bool CloseProcessRequest::Handle(MessageClient& client)
{
    client.Send(StatusResponse(true));

    return false;
}

//---------------------------------------------------------------------------
bool IsValidRequest::Handle(MessageClient& client)
{
    client.Send(StatusResponse(true));

    return true;
}

//---------------------------------------------------------------------------
bool ReadMemoryRequest::Handle(MessageClient& client)
{
    std::vector<uint8_t> buffer(GetSize());
    buffer.resize(GetSize());

    if (ReadMemory(GetAddress(), buffer))
    {
        client.Send(StatusWithValueResponse(true, std::move(buffer)));
    }
    else
    {
        client.Send(StatusWithValueResponse(false, {}));
    }

    return true;
}

//---------------------------------------------------------------------------
bool WriteMemoryRequest::Handle(MessageClient& client)
{
    const auto success = WriteMemory(const_cast<void*>(GetAddress()), GetData());

    client.Send(StatusResponse(success));

    return true;
}

//---------------------------------------------------------------------------
bool AllocateMemoryRequest::Handle(MessageClient& client)
{
    const auto address = reinterpret_cast<uintptr_t>(AllocateMemory(GetSize(), GetType(), GetProtect()));
    if (address != 0)
    {
        std::vector<uint8_t> addrVal;
        addrVal.resize(sizeof(address));
        *(uintptr_t*)&addrVal[0] = address;
        client.Send(StatusWithValueResponse(true, addrVal));
    }
    else
    {
        client.Send(StatusWithValueResponse(false, {}));
    }

    return true;
}

//---------------------------------------------------------------------------
bool FreeMemoryRequest::Handle(MessageClient& client)
{
    auto success = FreeMemory(reinterpret_cast<LPVOID>(GetAddress()));
    client.Send(StatusResponse(success));

    return true;
}

//---------------------------------------------------------------------------
bool ProtectMemoryRequest::Handle(MessageClient& client)
{
    auto success = ProtectMemory(reinterpret_cast<LPVOID>(GetAddress()), size, protect);

    client.Send(StatusResponse(success));

    return true;
}

//---------------------------------------------------------------------------
bool QueryMemoryRequest::Handle(MessageClient& client)
{
    auto protect = QueryMemory(reinterpret_cast<LPVOID>(GetAddress()));
    if (protect == 0)
    {
        client.Send(StatusWithValueResponse(false, {}));
    }
    else
    {
        std::vector<uint8_t> protectVal;
        protectVal.resize(sizeof(protect));
        *(uint32_t*)&protectVal[0] = protect;
        client.Send(StatusWithValueResponse(true, protectVal));
    }

    return true;
}

//---------------------------------------------------------------------------
bool CallFunctionRequest::Handle(MessageClient& client)
{
    auto addr = GetFunctionAddress();
    auto args = GetArguments();

    using SignalHandlerPointer = void(*)(int);

    SignalHandlerPointer previousHandler;
    previousHandler = signal(SIGSEGV, SignalHandler);
    std::string exception;
    auto ffiArgs = std::make_unique<ffi_type*[]>(args.size());
    auto values = std::make_unique<void*[]>(args.size());
    try
    {
        std::vector<uint8_t> retValue;
        ffi_cif cif;
        for (int i = 0; i < args.size(); i++)
        {
            ffiArgs[i] = &ffi_type_pointer;
            values[i] = &args[i];
        }

        auto res = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, static_cast<unsigned int>(args.size()),
                                &ffi_type_pointer, ffiArgs.get());
        if (res != FFI_OK)
        {
            std::stringstream ss;
            ss << "FFI prepare failed with code " << res << std::endl;
            std::cout << ss.str();
            throw std::exception(ss.str().c_str());
        }

        uintptr_t rc;
        ffi_call(&cif, FFI_FN(addr), &rc, values.get());
        retValue.resize(sizeof(rc));
        *(uintptr_t*)&retValue[0] = rc;
        client.Send(StatusWithValueResponse(true, retValue));

        signal(SIGSEGV, previousHandler);
        return true;
    }
    catch (const std::runtime_error& re)
    {
        exception = re.what();
    }
    catch (const std::exception& ex)
    {
        exception = ex.what();
    }
    catch (char* e)
    {
        exception = e;
    }
    catch (...)
    {
        exception = "Unknown exception";
    }

    std::cout << "Exception calling " << addr << ": " << exception << std::endl;
    client.Send(StatusWithValueResponse(false, std::vector<uint8_t>(exception.begin(), exception.end())));

    signal(SIGSEGV, previousHandler);
    return true;
}

//---------------------------------------------------------------------------
bool AllocateConsoleRequest::Handle(MessageClient& client)
{
    bool allocated = AllocConsole();
    if (allocated)
    {
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        std::cout.clear();
        std::clog.clear();
        std::cerr.clear();
        std::cin.clear();
    }

    client.Send(StatusResponse(allocated));

    return true;
}

//---------------------------------------------------------------------------
bool CreateFunctionRequest::Handle(MessageClient& client)
{
    std::string exception;
    try
    {
        auto functionAddress = AllocateRemoteFunction(GetName(), GetNumberOfArguments());
        if (functionAddress == 0)
        {
            throw std::exception("AllocateRemoteFunction returned zero");
        }
        std::vector<uint8_t> retValue(sizeof(functionAddress));
        *(uintptr_t*)&retValue[0] = functionAddress;
        client.Send(StatusWithValueResponse(true, retValue));
        std::cout << "Allocated function " << GetName() << " at " << std::hex << functionAddress << std::endl;
        return true;
    }
    catch (const std::runtime_error& re)
    {
        exception = re.what();
    }
    catch (const std::exception& ex)
    {
        exception = ex.what();
    }
    catch (char* e)
    {
        exception = e;
    }
    catch (...)
    {
        exception = "Unknown exception";
    }

    std::cout << "Exception while allocating function: " << exception << std::endl;
    client.Send(StatusWithValueResponse(false, std::vector<uint8_t>(exception.begin(), exception.end())));

    return true;
}

//---------------------------------------------------------------------------
bool FreeFunctionRequest::Handle(MessageClient& client)
{
    auto success = FreeRemoteFunction(name);
    if (success)
    {
        std::cout << "Freed function " << name << std::endl;
    }
    else
    {
        std::cout << "Failed to free " << name << std::endl;
    }
    client.Send(StatusResponse(success));
    return true;
}

//---------------------------------------------------------------------------
bool RemoteCallRequest::Handle(MessageClient& client)
{
    // This is mostly handled in the C# side.
    client.Send(RemoteCallResponse(0, 0));
    return true;
}
