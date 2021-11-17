#include <process.h>

#include "Client.hpp"
#include "MessageClient.hpp"
#include "RemoteFunctions.hpp"
#include "PipeStream/Exceptions.hpp"
#include "PipeStream/NamedPipeServerStream.hpp"

//---------------------------------------------------------------------------
[[noreturn]] void CommandThread(void*)
{
    const auto name = CreatePipeName("mbam-");

    while (true)
    {
        try
        {
            NamedPipeServerStream pipe(name, PipeDirection::InOut, 1, PipeTransmissionMode::Message);
            pipe.WaitForConnection();

            auto server = CreateClient(pipe);
            while (true)
            {
                auto message = server.Receive();
                if (message != nullptr)
                {
                    if (!message->Handle(server))
                    {
                        break;
                    }
                }
            }

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

//---------------------------------------------------------------------------
BOOL WINAPI DllMain(HMODULE handle, DWORD reason, PVOID reversed)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        _beginthread(CommandThread, 0, nullptr);
        _beginthread(RemoteFunctionDataThread, 0, nullptr);
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
