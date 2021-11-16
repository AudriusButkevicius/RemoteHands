using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO;
using System.IO.Pipes;
using System.Text;

namespace RemoteHands
{
    internal class MessageClient : IDisposable
    {
        private readonly PipeStream _pipe;

        private readonly Dictionary<MessageType, Func<IMessage>> _registeredMessages =
            new Dictionary<MessageType, Func<IMessage>>();

        public MessageClient(PipeStream pipe)
        {
            Contract.Requires(pipe != null);

            _pipe = pipe;
        }

        public IntPtr Id => _pipe.SafePipeHandle.DangerousGetHandle();

        public void Dispose()
        {
            _pipe?.Dispose();
        }

        public void RegisterMessage<T>() where T : IMessage, new()
        {
            IMessage MessageCreator()
            {
                return new T();
            }

            _registeredMessages.Add(MessageCreator().MessageType, MessageCreator);
        }

        public IMessage Receive()
        {
            using (var ms = new MemoryStream())
            {
                var buffer = new byte[256];
                do
                {
                    int length = _pipe.Read(buffer, 0, buffer.Length);
                    ms.Write(buffer, 0, length);
                } while (!_pipe.IsMessageComplete);

                ms.Position = 0;

                using (var br = new BinaryReader(ms, Encoding.Unicode))
                {
                    var type = (MessageType)br.ReadInt32();

                    if (_registeredMessages.TryGetValue(type, out Func<IMessage> createFn))
                    {
                        IMessage message = createFn();
                        message.ReadFrom(br);
                        return message;
                    }
                }
            }

            return null;
        }

        public void Send(IMessage message)
        {
            Contract.Requires(message != null);

            using (var ms = new MemoryStream())
            {
                using (var bw = new BinaryWriter(ms, Encoding.Unicode))
                {
                    bw.Write((int)message.MessageType);
                    message.WriteTo(bw);
                }

                byte[] buffer = ms.ToArray();
                _pipe.Write(buffer, 0, buffer.Length);
            }
        }
    }
}