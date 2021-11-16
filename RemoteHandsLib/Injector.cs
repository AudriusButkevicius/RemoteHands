using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace RemoteHands
{
    public static class Injector
    {
        // privileges
        private const int PROCESS_CREATE_THREAD = 0x0002;
        private const int PROCESS_QUERY_INFORMATION = 0x0400;
        private const int PROCESS_VM_OPERATION = 0x0008;
        private const int PROCESS_VM_WRITE = 0x0020;
        private const int PROCESS_VM_READ = 0x0010;

        // used for memory allocation/freeing
        private const uint MEM_COMMIT = 0x00001000;
        private const uint MEM_RESERVE = 0x00002000;
        private const uint MEM_RELEASE = 0x00008000;
        private const uint PAGE_READWRITE = 4;

        // wait timeout
        private const uint INFINITE = 0xFFFFFFFF;

        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        private static extern IntPtr VirtualAllocEx(
            IntPtr hProcess, IntPtr lpAddress,
            uint dwSize, uint flAllocationType, uint flProtect
        );

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        private static extern bool VirtualFreeEx(
            IntPtr hProcess, IntPtr lpAddress,
            uint dwSize, uint dwFreeType
        );

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(
            IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer,
            uint nSize,
            out UIntPtr lpNumberOfBytesWritten
        );

        [DllImport("kernel32.dll")]
        private static extern IntPtr CreateRemoteThread(
            IntPtr hProcess,
            IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter,
            uint dwCreationFlags, IntPtr lpThreadId
        );

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern uint WaitForSingleObject(IntPtr hHandle, uint dwMilliseconds);

        public static bool Inject(int pid, string dllName)
        {
            if (!File.Exists(dllName))
            {
                throw new FileLoadException("File does not exist", dllName);
            }

            // the target process - I'm using a dummy process for this
            // if you don't have one, open Task Manager and choose wisely
            var targetProcess = Process.GetProcessById(pid);

            foreach (ProcessModule targetProcessModule in targetProcess.Modules)
            {
                if (targetProcessModule.FileName == dllName)
                {
                    return false;
                }
            }

            // geting the handle of the process - with required privileges
            IntPtr procHandle =
                OpenProcess(
                    PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
                    PROCESS_VM_WRITE | PROCESS_VM_READ, false, targetProcess.Id
                );

            try
            {
                // searching for the address of LoadLibraryA and storing it in a pointer
                IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                var sz = (uint)((dllName.Length + 1) * Marshal.SizeOf(typeof(char)));

                // alocating some memory on the target process - enough to store the name of the dll
                // and storing its address in a pointer
                IntPtr allocMemAddress =
                    VirtualAllocEx(procHandle, IntPtr.Zero, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                // writing the name of the dll there
                WriteProcessMemory(procHandle, allocMemAddress, Encoding.Default.GetBytes(dllName), sz, out _);

                // creating a thread that will call LoadLibraryA with allocMemAddress as argument
                IntPtr threadHandle = CreateRemoteThread(
                    procHandle, IntPtr.Zero, 0, loadLibraryAddr, allocMemAddress,
                    0, IntPtr.Zero
                );

                WaitForSingleObject(threadHandle, INFINITE);

                CloseHandle(threadHandle);

                VirtualFreeEx(procHandle, allocMemAddress, 0, MEM_RELEASE);

                return true;
            }
            finally
            {
                CloseHandle(procHandle);
            }
        }
    }
}