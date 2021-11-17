#include <algorithm>
#include <cstdint>
#include <vector>
#include <windows.h>

bool IsValidMemoryRange(LPCVOID address, int length)
{
    const auto endAddress = static_cast<const uint8_t*>(address) + length;

    do
    {
        MEMORY_BASIC_INFORMATION info;
        if (!VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION)))
        {
            return false;
        }

        if (info.State != MEM_COMMIT)
        {
            return false;
        }

        switch (info.Protect)
        {
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
        case PAGE_READONLY:
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
            break;
        default:
            return false;
        }

        address = static_cast<uint8_t*>(info.BaseAddress) + info.RegionSize;
    }
    while (endAddress > address);
    return true;
}

//---------------------------------------------------------------------------
LPVOID AllocateMemory(uint32_t size, uint32_t type, uint32_t protect)
{
    return VirtualAlloc(nullptr, size, type, protect);
}

//---------------------------------------------------------------------------
bool FreeMemory(LPVOID address)
{
    if (!IsValidMemoryRange(address, 0))
    {
        return false;
    }

    return VirtualFree(address, 0, MEM_RELEASE);
}

//---------------------------------------------------------------------------
bool ReadMemory(LPCVOID address, std::vector<uint8_t>& buffer)
{
    if (!IsValidMemoryRange(address, static_cast<int>(buffer.size())))
    {
        return false;
    }

    std::memcpy(buffer.data(), address, buffer.size());

    return true;
}

//---------------------------------------------------------------------------
bool WriteMemory(LPVOID address, const std::vector<uint8_t>& buffer)
{
    if (!IsValidMemoryRange(address, static_cast<int>(buffer.size())))
    {
        return false;
    }

    std::memcpy(address, buffer.data(), buffer.size());
    return true;
}

//---------------------------------------------------------------------------
bool ProtectMemory(LPVOID address, uint32_t size, uint32_t protect)
{
    if (!IsValidMemoryRange(address, static_cast<int>(size)))
    {
        return false;
    }

    return VirtualProtect(address, size, protect, nullptr);
}

//---------------------------------------------------------------------------
uint32_t QueryMemory(LPVOID address)
{
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION)))
    {
        return 0;
    }
    return info.Protect;
}
