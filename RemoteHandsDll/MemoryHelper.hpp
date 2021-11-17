#include <windows.h>
#include <vector>
#include <cstdint>

bool IsValidMemoryRange(LPCVOID address, int length);

LPVOID AllocateMemory(uint32_t size, uint32_t type, uint32_t protect);

bool FreeMemory(LPVOID address);

bool ReadMemory(LPCVOID address, std::vector<uint8_t>& buffer);

bool WriteMemory(LPVOID address, const std::vector<uint8_t>& buffer);

bool ProtectMemory(LPVOID address, uint32_t size, uint32_t protect);

uint32_t QueryMemory(LPVOID address);
