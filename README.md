# RemoteHands

A small generic library for remote process manipulation.

The project is composed out of two parts, a C++ DLL you inject into a remote process, and a C# library that you then use
to manipulate the process in which the DLL has been injected.

After having injected the DLL, you can then perform the following from C#:

1. Read/Write/Allocate/Free/Query/Protect memory in the remote process
2. Call arbitrary functions in the remote process from the C# library.
4. Create new "native" functions in the remote process, who's calls are received in C#.

The C# library contains basic code allowing to perform DLL injection from C#.

## How is this useful

The project started because I wanted to avoid having to write C++ to develop a game bot I was working on.

This is mostly because I have not done much C++ in my life, and the development cycle was quite slow, namely you usually
have to restart the game to inject a new version of the DLL with the updated logic.

The memory manipulation aspect of this library is generally not that useful, as you can already do that in Windows
using `Virtual*Ex` system calls, however because I based this project on a project that already had ability to do memory
reading/writing, it made sense to keep it, and add ability to allocate/free/query/protect.

Using `Virtual*Ex` system calls, you do however require to have a handle open on the remote process, which might make it
easier to detect.

This doesn't however mean that using this library is undetectable because there are no handles open to the remote
process. The remote process could simply list loaded modules/DLLs, and find the injected DLL, or look for it's open file
descriptors and find unusual named pipes open.

There are more advanced ways to inject DLLs that would not come up in the processes module list, but it is beyond the
scope of this project.

## How does it work

After injecting the C++ DLL, the DLL opens two named pipes.

One named pipe is for receiving commands, i.e, allocating memory, creating functions, etc, and flows in C# -> C++
direction. Second name pipe is used for forwarding function calls that happen in the functions created in the native
process, to the remote process using the C# library, i.e, C++ -> C#.

## Known caveats

1. If you patch out a function to call your function, and then your process hosting the C# library crashes, your
   function will now not do anything and return 0/null for all calls. If you are patching out remote functions,
   recommend you have finalizers that restore the patches on exit/crash.
2. Calls to functions created by this library are serialized, because we need to serialize access to the pipe, and
   preserve request->response pattern. This could be improved in the future by giving request ids to function calls, for
   which responses could be received asynchronously.
3. The library naively assumes that all arguments to all functions are of type pointer (8 bytes). This works for most
   things, but most likely will not work for things that expect structs, as I am not sure what the calling convention
   for that looks like. libffi which is used for most of the calling/function creating logic does support structs, so
   this could be added if needed.
4. I'm not a C++ developer, so there is probably wonky C++ code all over the place that will crash make the world burn.
5. There is a good chunk of global state, because I think you have to have it.

## Dependencies

The project expects you to have vcpkg installed and available on the PATH, as that is used for C++ dependency
management.

### C++

* libffi
* boost-stacktrace

### C#

None

## Building

Project has been tested to build using the latest version of Visual Studio 2019 with C++ and C# SDKs, but newer versions
might work too.

Depending on your development environment, you might be able to build the project using a simple `dotnet build` if the
SDKs are available.

The project has also been tested to build using Rider and Rider for Unreal Engine.

## Inspiration

This was heavily based on https://github.com/ReClassNET/ReClass.NET-MemoryPipePlugin
Kudos to them for the inspiration.
