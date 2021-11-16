#pragma once

#include <string>
#include <utility>
#include <iostream>

#include "PipeStream/BinaryReader.hpp"
#include "PipeStream/BinaryWriter.hpp"

class MessageClient;

enum class MessageType
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
    RemoteCallResponse = 16,
};

//---------------------------------------------------------------------------
class IMessage
{
public:
    virtual ~IMessage() = default;

    [[nodiscard]] virtual MessageType GetMessageType() const = 0;

    virtual void ReadFrom(BinaryReader& br) = 0;

    virtual void WriteTo(BinaryWriter& bw) const = 0;

    virtual bool Handle(MessageClient& client)
    {
        return true;
    }
};

//---------------------------------------------------------------------------
class StatusResponse : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::StatusResponse; }

    [[nodiscard]] bool GetSuccess() const { return success; }

    StatusResponse()
        : success(false)
    {
    }

    explicit StatusResponse(bool _success)
        : success(_success)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        success = reader.ReadBoolean();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(success);
    }

private:
    bool success;
};

//---------------------------------------------------------------------------
class StatusWithValueResponse : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::StatusWithValueResponse; }

    [[nodiscard]] bool GetSuccess() const { return success; }

    [[nodiscard]] std::vector<uint8_t> GetValue() const { return value; }

    StatusWithValueResponse()
        : success(false),
          value()
    {
    }

    StatusWithValueResponse(bool _success, std::vector<uint8_t> _value)
        : success(_success),
          value(std::move(_value))
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        success = reader.ReadBoolean();
        const auto size = reader.ReadUInt32();
        value = reader.ReadBytes(size);
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(success);
        const auto size = static_cast<int>(value.size());
        writer.Write(size);
        writer.Write(value.data(), 0, size);
    }

private:
    bool success;
    std::vector<uint8_t> value;
};

//---------------------------------------------------------------------------
class CloseProcessRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::CloseProcessRequest; }

    void ReadFrom(BinaryReader& reader) override
    {
    }

    void WriteTo(BinaryWriter& writer) const override
    {
    }

    bool Handle(MessageClient& client) override;
};

//---------------------------------------------------------------------------
class IsValidRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::IsValidRequest; }

    void ReadFrom(BinaryReader& reader) override
    {
    }

    void WriteTo(BinaryWriter& writer) const override
    {
    }

    bool Handle(MessageClient& client) override;
};

//---------------------------------------------------------------------------
class ReadMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::ReadMemoryRequest; }

    [[nodiscard]] const void* GetAddress() const { return address; }

    [[nodiscard]] int GetSize() const { return size; }

    ReadMemoryRequest()
        : address(nullptr),
          size(0)
    {
    }

    ReadMemoryRequest(const void* _address, int _size)
        : address(_address),
          size(_size)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        address = reader.ReadIntPtr();
        size = reader.ReadInt32();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(address);
        writer.Write(size);
    }

    bool Handle(MessageClient& client) override;

private:
    const void* address;
    int size;
};

//---------------------------------------------------------------------------
class WriteMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::WriteMemoryRequest; }

    [[nodiscard]] const void* GetAddress() const { return address; }

    [[nodiscard]] const std::vector<uint8_t>& GetData() const { return data; }

    WriteMemoryRequest()
        : address(nullptr)
    {
    }

    WriteMemoryRequest(const void* _address, std::vector<uint8_t>&& _data)
        : address(_address),
          data(std::move(_data))
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        address = reader.ReadIntPtr();
        data = reader.ReadBytes(reader.ReadInt32());
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(address);
        writer.Write(static_cast<int>(data.size()));
        writer.Write(data.data(), 0, static_cast<int>(data.size()));
    }

    bool Handle(MessageClient& client) override;

private:
    const void* address;
    std::vector<uint8_t> data;
};

//---------------------------------------------------------------------------
class AllocateMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::AllocateMemoryRequest; }

    [[nodiscard]] uint32_t GetSize() const { return size; }

    [[nodiscard]] uint32_t GetType() const { return type; }

    [[nodiscard]] uint32_t GetProtect() const { return protect; }

    AllocateMemoryRequest()
        : size(0), type(0), protect(0)
    {
    }

    AllocateMemoryRequest(uint32_t _size, uint32_t _type, uint32_t _protect)
        : size(_size),
          type(_type),
          protect(_protect)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        size = reader.ReadUInt32();
        type = reader.ReadUInt32();
        protect = reader.ReadUInt32();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(size);
        writer.Write(type);
        writer.Write(protect);
    }

    bool Handle(MessageClient& client) override;

private:
    uint32_t size;
    uint32_t type;
    uint32_t protect;
};

//---------------------------------------------------------------------------
class FreeMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::FreeMemoryRequest; }

    [[nodiscard]] uintptr_t GetAddress() const { return address; }

    FreeMemoryRequest()
        : address(0)
    {
    }

    explicit FreeMemoryRequest(uintptr_t _address) : address(_address)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        address = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(address);
    }

    bool Handle(MessageClient& client) override;

private:
    uintptr_t address;
};

//---------------------------------------------------------------------------
class QueryMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::QueryMemoryRequest; }

    [[nodiscard]] uintptr_t GetAddress() const { return address; }

    QueryMemoryRequest()
        : address(0)
    {
    }

    explicit QueryMemoryRequest(uintptr_t _address) : address(_address)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        address = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(address);
    }

    bool Handle(MessageClient& client) override;

private:
    uintptr_t address;
};

//---------------------------------------------------------------------------
class ProtectMemoryRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::ProtectMemoryRequest; }

    [[nodiscard]] uintptr_t GetAddress() const { return address; }

    ProtectMemoryRequest()
        : address(0), size(0), protect(0)
    {
    }

    explicit ProtectMemoryRequest(uintptr_t _address, uint32_t _size, uint32_t _protect) :
        address(_address), size(_size), protect(_protect)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        address = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
        size = reader.ReadUInt32();
        protect = reader.ReadUInt32();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(address);
        writer.Write(size);
        writer.Write(protect);
    }

    bool Handle(MessageClient& client) override;

private:
    uintptr_t address;
    uint32_t size;
    uint32_t protect;
};

//---------------------------------------------------------------------------
class CallFunctionRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::CallFunctionRequest; }

    [[nodiscard]] std::vector<uintptr_t> GetArguments() const { return arguments; }

    [[nodiscard]] uintptr_t GetFunctionAddress() const { return functionAddress; }

    CallFunctionRequest()
        : functionAddress(0),
          arguments()
    {
    }

    CallFunctionRequest(uintptr_t _functionAddress,
                        std::vector<uintptr_t> _arguments)
        : functionAddress(_functionAddress),
          arguments(std::move(_arguments))
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        functionAddress = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
        arguments.resize(reader.ReadInt32());
        for (auto& argument : arguments)
        {
            argument = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
        }
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(reinterpret_cast<void*>(functionAddress));
        writer.Write(static_cast<int>(arguments.size()));
        for (auto arg : arguments)
        {
            writer.Write(reinterpret_cast<void*>(arg));
        }
    }

    bool Handle(MessageClient& client) override;

private:
    uintptr_t functionAddress;
    std::vector<uintptr_t> arguments;
};

//---------------------------------------------------------------------------
class AllocateConsoleRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::AllocateConsoleRequest; }

    void ReadFrom(BinaryReader& reader) override
    {
    }

    void WriteTo(BinaryWriter& writer) const override
    {
    }

    bool Handle(MessageClient& client) override;
};

//---------------------------------------------------------------------------
class CreateFunctionRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::CreateFunctionRequest; }

    [[nodiscard]] std::string GetName() const { return name; }

    [[nodiscard]] uint32_t GetNumberOfArguments() const { return numArgs; }

    CreateFunctionRequest()
        : name(),
          numArgs()
    {
    }

    CreateFunctionRequest(std::string _name, uint32_t _numArgs) : name(std::move(_name)), numArgs(_numArgs)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        name = reader.ReadString();
        numArgs = reader.ReadUInt32();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(name);
        writer.Write(numArgs);
    }

    bool Handle(MessageClient& client) override;

private:
    std::string name;
    uint32_t numArgs;
};

//---------------------------------------------------------------------------
class FreeFunctionRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::FreeFunctionRequest; }

    FreeFunctionRequest() : name()
    {
    }

    explicit FreeFunctionRequest(std::string _name) : name(std::move(_name))
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
    }

    void WriteTo(BinaryWriter& writer) const override
    {
    }

    bool Handle(MessageClient& client) override;

private:
    std::string name;
};


//---------------------------------------------------------------------------
class RemoteCallRequest : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::RemoteCallRequest; }


    RemoteCallRequest()
        : name(),
          arguments()
    {
    }

    RemoteCallRequest(std::string _name, std::vector<uintptr_t> args) : name(std::move(_name)),
                                                                        arguments(std::move(args))
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        name = reader.ReadString();
        int numArguments = reader.ReadInt32();
        arguments.resize(numArguments);
        for (int i = 0; i < numArguments; i++)
        {
            arguments[i] = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
        }
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(name);
        writer.Write(static_cast<int>(arguments.size()));
        for (const auto& item : arguments)
        {
            writer.Write(reinterpret_cast<void*>(item));
        }
    }

    bool Handle(MessageClient& client) override;

private:
    std::string name;
    std::vector<uintptr_t> arguments;
};

//---------------------------------------------------------------------------
class RemoteCallResponse : public IMessage
{
public:
    [[nodiscard]] MessageType GetMessageType() const override { return MessageType::RemoteCallResponse; }

    [[nodiscard]] uintptr_t GetReturnValue() const { return returnValue; }

    [[nodiscard]] uint32_t GetFlags() const { return flags; }

    RemoteCallResponse() : returnValue(0), flags(0)
    {
    }

    RemoteCallResponse(uintptr_t _returnValue, uint32_t _flags) : returnValue(_returnValue), flags(_flags)
    {
    }

    void ReadFrom(BinaryReader& reader) override
    {
        returnValue = reinterpret_cast<uintptr_t>(reader.ReadIntPtr());
        flags = reader.ReadUInt32();
    }

    void WriteTo(BinaryWriter& writer) const override
    {
        writer.Write(reinterpret_cast<void*>(returnValue));
        writer.Write(flags);
    }

private:
    uintptr_t returnValue;
    uint32_t flags;
};
