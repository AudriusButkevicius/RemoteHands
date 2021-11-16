using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO;

namespace RemoteHands
{
    internal enum MessageType
    {
        StatusResponse = 1,
        StatusWithValueResponse = 2,
        CloseProcessRequest = 3,
        IsValidRequest = 4,
        ReadMemoryRequest = 5,
        WriteMemoryRequest = 6,
        AllocateMemoryRequest = 7,
        FreeMemoryRequest = 8,
        QueryMemoryRequest = 9,
        ProtectMemoryRequest = 10,
        CallFunctionRequest = 11,
        AllocateConsoleRequest = 12,
        CreateFunctionRequest = 13,
        FreeFunctionRequest = 14,
        RemoteCallRequest = 15,
        RemoteCallResponse = 16
    }

    internal interface IMessage
    {
        MessageType MessageType { get; }

        void ReadFrom(BinaryReader reader);
        void WriteTo(BinaryWriter writer);
    }

    [ContractClassFor(typeof(IMessage))]
    internal class MessageContract : IMessage
    {
        public MessageType MessageType => throw new NotImplementedException();

        public void ReadFrom(BinaryReader reader)
        {
            Contract.Requires(reader != null);

            throw new NotImplementedException();
        }

        public void WriteTo(BinaryWriter writer)
        {
            Contract.Requires(writer != null);

            throw new NotImplementedException();
        }
    }

    internal class StatusResponse : IMessage
    {
        public StatusResponse()
        {
        }

        public StatusResponse(bool success)
        {
            Success = success;
        }

        public bool Success { get; private set; }
        public MessageType MessageType => MessageType.StatusResponse;

        public void ReadFrom(BinaryReader reader)
        {
            Success = reader.ReadBoolean();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Success);
        }
    }

    internal class StatusWithValueResponse : IMessage
    {
        public StatusWithValueResponse()
        {
        }

        public StatusWithValueResponse(bool success, byte[] value)
        {
            Success = success;
            Value = value;
        }

        public bool Success { get; private set; }
        public byte[] Value { get; private set; }
        public MessageType MessageType => MessageType.StatusWithValueResponse;

        public void ReadFrom(BinaryReader reader)
        {
            Success = reader.ReadBoolean();
            Value = reader.ReadBytes(reader.ReadInt32());
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Success);
            writer.Write(Value.Length);
            writer.Write(Value);
        }
    }

    internal class CloseProcessRequest : IMessage
    {
        public MessageType MessageType => MessageType.CloseProcessRequest;

        public void ReadFrom(BinaryReader reader)
        {
        }

        public void WriteTo(BinaryWriter writer)
        {
        }
    }

    internal class IsValidRequest : IMessage
    {
        public MessageType MessageType => MessageType.IsValidRequest;

        public void ReadFrom(BinaryReader reader)
        {
        }

        public void WriteTo(BinaryWriter writer)
        {
        }
    }

    internal class ReadMemoryRequest : IMessage
    {
        public ReadMemoryRequest()
        {
        }

        public ReadMemoryRequest(IntPtr address, int size)
        {
            Address = address;
            Size = size;
        }

        public IntPtr Address { get; private set; }
        public int Size { get; private set; }
        public MessageType MessageType => MessageType.ReadMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Address = reader.ReadIntPtr();
            Size = reader.ReadInt32();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Address);
            writer.Write(Size);
        }
    }

    internal class WriteMemoryRequest : IMessage
    {
        public WriteMemoryRequest()
        {
        }

        public WriteMemoryRequest(IntPtr address, byte[] data)
        {
            Address = address;
            Data = data;
        }

        public IntPtr Address { get; private set; }
        public byte[] Data { get; private set; }
        public MessageType MessageType => MessageType.WriteMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Address = reader.ReadIntPtr();
            int size = reader.ReadInt32();
            Data = reader.ReadBytes(size);
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Address);
            writer.Write(Data.Length);
            writer.Write(Data);
        }
    }

    internal class AllocateMemoryRequest : IMessage
    {
        public AllocateMemoryRequest()
        {
        }

        public AllocateMemoryRequest(
            int size, MemoryAllocationType type,
            MemoryProtectionConstraints protectionConstraints
        )
        {
            Size = size;
            AllocationType = type;
            ProtectionConstraints = protectionConstraints;
        }

        public int Size { get; private set; }
        public MemoryAllocationType AllocationType { get; private set; }
        public MemoryProtectionConstraints ProtectionConstraints { get; private set; }
        public MessageType MessageType => MessageType.AllocateMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Size = (int)reader.ReadUInt32();
            AllocationType = (MemoryAllocationType)reader.ReadUInt32();
            ProtectionConstraints = (MemoryProtectionConstraints)reader.ReadUInt32();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write((uint)Size);
            writer.Write((uint)AllocationType);
            writer.Write((uint)ProtectionConstraints);
        }
    }

    internal class FreeMemoryRequest : IMessage
    {
        public FreeMemoryRequest()
        {
        }

        public FreeMemoryRequest(IntPtr address)
        {
            Address = address;
        }

        public IntPtr Address { get; private set; }
        public MessageType MessageType => MessageType.FreeMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Address = reader.ReadIntPtr();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Address);
        }
    }

    internal class QueryMemoryRequest : IMessage
    {
        public QueryMemoryRequest()
        {
        }

        public QueryMemoryRequest(IntPtr address)
        {
            Address = address;
        }

        public IntPtr Address { get; private set; }
        public MessageType MessageType => MessageType.QueryMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Address = reader.ReadIntPtr();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Address);
        }
    }

    internal class ProtectMemoryRequest : IMessage
    {
        public ProtectMemoryRequest()
        {
        }

        public ProtectMemoryRequest(IntPtr address, uint size, MemoryProtectionConstraints protectionConstraints)
        {
            Address = address;
            Size = size;
            ProtectionConstraints = protectionConstraints;
        }

        public IntPtr Address { get; private set; }
        public uint Size { get; private set; }
        public MemoryProtectionConstraints ProtectionConstraints { get; private set; }
        public MessageType MessageType => MessageType.ProtectMemoryRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Address = reader.ReadIntPtr();
            Size = reader.ReadUInt32();
            ProtectionConstraints = (MemoryProtectionConstraints)reader.ReadUInt32();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Address);
            writer.Write(Size);
            writer.Write((uint)ProtectionConstraints);
        }
    }

    internal class CallFunctionRequest : IMessage
    {
        public CallFunctionRequest()
        {
        }

        public CallFunctionRequest(IntPtr functionAddress, List<IntPtr> arguments = null)
        {
            FunctionAddress = functionAddress;
            Arguments = arguments ?? new List<IntPtr>();
        }

        public IntPtr FunctionAddress { get; private set; }
        public List<IntPtr> Arguments { get; private set; }
        public MessageType MessageType => MessageType.CallFunctionRequest;

        public void ReadFrom(BinaryReader reader)
        {
            FunctionAddress = reader.ReadIntPtr();
            Arguments = new List<IntPtr>(reader.ReadInt32());
            for (var i = 0; i < Arguments.Capacity; i++)
            {
                Arguments.Add(reader.ReadIntPtr());
            }
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(FunctionAddress);
            writer.Write(Arguments.Count);
            foreach (IntPtr arg in Arguments)
            {
                writer.Write(arg);
            }
        }

        public void AddArgument(IntPtr value)
        {
            Arguments.Add(value);
        }
    }

    internal class AllocateConsoleRequest : IMessage
    {
        public MessageType MessageType => MessageType.AllocateConsoleRequest;

        public void ReadFrom(BinaryReader reader)
        {
        }

        public void WriteTo(BinaryWriter writer)
        {
        }
    }

    internal class CreateFunctionRequest : IMessage
    {
        public CreateFunctionRequest()
        {
        }

        public CreateFunctionRequest(string name, uint numberOfArguments)
        {
            Name = name;
            NumberOfArguments = numberOfArguments;
        }

        public string Name { get; private set; }

        public uint NumberOfArguments { get; private set; }
        public MessageType MessageType => MessageType.CreateFunctionRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Name = reader.ReadString();
            NumberOfArguments = reader.ReadUInt32();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Name);
            writer.Write(NumberOfArguments);
        }
    }

    internal class FreeFunctionRequest : IMessage
    {
        public FreeFunctionRequest()
        {
        }

        public FreeFunctionRequest(string name)
        {
            Name = name;
        }

        public string Name { get; private set; }
        public MessageType MessageType => MessageType.FreeFunctionRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Name = reader.ReadString();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Name);
        }
    }

    internal class RemoteCallRequestMessage : IMessage
    {
        public RemoteCallRequestMessage()
        {
        }

        public RemoteCallRequestMessage(string name, List<IntPtr> arguments)
        {
            Name = name;
            Arguments = arguments;
        }

        public string Name { get; private set; }

        public List<IntPtr> Arguments { get; private set; }
        public MessageType MessageType => MessageType.RemoteCallRequest;

        public void ReadFrom(BinaryReader reader)
        {
            Name = reader.ReadString();
            Arguments = new List<IntPtr>(reader.ReadInt32());
            for (var i = 0; i < Arguments.Capacity; i++)
            {
                Arguments.Add(reader.ReadIntPtr());
            }
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(Name);
            writer.Write(Arguments.Count);
            foreach (IntPtr argumentPointer in Arguments)
            {
                writer.Write(argumentPointer);
            }
        }
    }

    internal class RemoteCallResponseMessage : IMessage
    {
        public RemoteCallResponseMessage()
        {
        }

        public RemoteCallResponseMessage(IntPtr returnValue, ResponseFlags flags)
        {
            ReturnValue = returnValue;
            Flags = flags;
        }

        public IntPtr ReturnValue { get; private set; }

        public ResponseFlags Flags { get; private set; }
        public MessageType MessageType => MessageType.RemoteCallResponse;

        public void ReadFrom(BinaryReader reader)
        {
            ReturnValue = reader.ReadIntPtr();
            Flags = (ResponseFlags)reader.ReadUInt32();
        }

        public void WriteTo(BinaryWriter writer)
        {
            writer.Write(ReturnValue);
            writer.Write((uint)Flags);
        }
    }
}