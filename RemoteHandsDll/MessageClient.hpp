#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "Messages.hpp"
#include "PipeStream/PipeStream.hpp"

class MessageClient
{
public:
    explicit MessageClient(PipeStream& pipe);

    template <typename T>
    void RegisterMessage()
    {
        const auto messageCreator = []() { return std::make_unique<T>(); };

        registeredMessages[messageCreator()->GetMessageType()] = messageCreator;
    }

    virtual std::unique_ptr<IMessage> Receive();

    virtual void Send(const IMessage& message) const;

private:
    PipeStream& pipe;

    std::unordered_map<MessageType, std::function<std::unique_ptr<IMessage>()>> registeredMessages;
};
