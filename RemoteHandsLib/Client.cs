using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading;

namespace RemoteHands
{
    public class Client : IDisposable
    {
        private static readonly Dictionary<int, Client> _clients = new Dictionary<int, Client>();

        private readonly MessageClient _client;
        private readonly MessageClient _functionClient;
        private readonly Dictionary<string, CallHandler> _functions = new Dictionary<string, CallHandler>();
        private readonly Thread _thread;

        private Client(int pid)
        {
            _client = CreateClient("mbam", pid);
            _functionClient = CreateClient("mbamd", pid);
            _thread = new Thread(ReadFunctionCalls)
            {
                IsBackground = true
            };
            _thread.Start();
        }

        public void Dispose()
        {
            foreach (string name in _functions.Keys)
            {
                FreeFunction(name);
            }

            _functions.Clear();
            _client?.Dispose();
            _thread.Abort();
            _thread.Join();
            _functionClient?.Dispose();
        }

        public static Client Create(int pid)
        {
            lock (_clients)
            {
                if (!_clients.TryGetValue(pid, out Client client))
                {
                    client = new Client(pid);
                    _clients[pid] = client;
                }

                return client;
            }
        }

        private void ReadFunctionCalls()
        {
            while (true)
            {
                var msg = _functionClient.Receive() as RemoteCallRequestMessage;
                try
                {
                    CallHandler handler = _functions[msg.Name];
                    CallResponse response = handler.Invoke(msg.Arguments);
                    _functionClient.Send(new RemoteCallResponseMessage(response.ReturnValue, response.Falgs));
                }
                catch (ThreadAbortException)
                {
                    _functionClient.Send(new RemoteCallResponseMessage());
                    _functionClient.Dispose();
                    return;
                }
                catch (Exception)
                {
                    _functionClient.Send(new RemoteCallResponseMessage());
                }
            }
        }

        private MessageClient CreateClient(string name, int pid)
        {
            var pipeName = $"{name}-{pid}";
            var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut);
            pipe.Connect();
            pipe.ReadMode = PipeTransmissionMode.Message;

            var client = new MessageClient(pipe);

            client.RegisterMessage<StatusResponse>();
            client.RegisterMessage<StatusWithValueResponse>();
            client.RegisterMessage<RemoteCallRequestMessage>();

            return client;
        }

        public bool IsValid()
        {
            lock (this)
            {
                _client.Send(new IsValidRequest());
                var message = _client.Receive() as StatusResponse;

                return message.Success;
            }
        }

        public void Close()
        {
            lock (this)
            {
                try
                {
                    _client.Send(new CloseProcessRequest());
                    _client.Receive(); // swallow the StatusResponse
                }
                catch
                {
                }

                _client.Dispose();
            }
        }

        public bool ReadMemory(IntPtr address, ref byte[] buffer, int offset, int size)
        {
            lock (this)
            {
                _client.Send(new ReadMemoryRequest(address, size));
                var response = _client.Receive() as StatusWithValueResponse;
                if (!response.Success)
                {
                    return false;
                }

                if (response.Value.Length != size)
                {
                    return false;
                }

                Array.Copy(response.Value, 0, buffer, offset, size);

                return true;
            }
        }

        public IntPtr AllocateMemory(
            int size, MemoryAllocationType allocationType,
            MemoryProtectionConstraints protectionConstraints
        )
        {
            lock (this)
            {
                _client.Send(new AllocateMemoryRequest(size, allocationType, protectionConstraints));
                var response = _client.Receive() as StatusWithValueResponse;
                if (!response.Success)
                {
                    throw new ApplicationException("Failed to allocate");
                }

                if (response.Value.Length != 8)
                {
                    throw new InvalidDataException("Non 8 bytes");
                }

                return new IntPtr(BitConverter.ToInt64(response.Value, 0));
            }
        }

        public bool FreeMemory(IntPtr addr)
        {
            lock (this)
            {
                _client.Send(new FreeMemoryRequest(addr));
                var response = _client.Receive() as StatusResponse;
                return response.Success;
            }
        }

        public bool WriteMemory(IntPtr address, ref byte[] buffer, int offset, int size)
        {
            lock (this)
            {
                var data = new byte[size];
                Array.Copy(buffer, offset, data, 0, size);

                _client.Send(new WriteMemoryRequest(address, data));
                var message = _client.Receive() as StatusResponse;
                return message.Success;
            }
        }

        public bool ProtectMemory(IntPtr address, int size, MemoryProtectionConstraints protectionConstraints)
        {
            lock (this)
            {
                _client.Send(new ProtectMemoryRequest(address, (uint)size, protectionConstraints));
                var message = _client.Receive() as StatusResponse;
                return message.Success;
            }
        }

        public MemoryProtectionConstraints QueryMemory(IntPtr address)
        {
            lock (this)
            {
                _client.Send(new QueryMemoryRequest(address));
                var message = _client.Receive() as StatusWithValueResponse;
                if (!message.Success)
                {
                    throw new IOException("Failed to query memory");
                }

                return (MemoryProtectionConstraints)BitConverter.ToUInt32(message.Value, 0);
            }
        }

        public bool AllocateConsole()
        {
            lock (this)
            {
                _client.Send(new AllocateConsoleRequest());
                var message = _client.Receive() as StatusResponse;
                return message.Success;
            }
        }

        public IntPtr CallFunction(IntPtr address, params object[] args)
        {
            lock (this)
            {
                // When we are passed an array as value of a specific type.
                if (args.Length == 1 && args[0].GetType().IsArray)
                {
                    args = (args[0] as IEnumerable).Cast<object>().ToArray();
                }

                List<IntPtr> arguments = new List<IntPtr>(args.Length);
                foreach (object arg in args)
                {
                    switch (arg)
                    {
                        case bool val:
                            arguments.Add(new IntPtr(val ? 1 : 0));
                            break;
                        case byte val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case short val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case ushort val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case uint val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case int val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case long val:
                            arguments.Add(new IntPtr(val));
                            break;
                        case ulong val:
                            arguments.Add(new IntPtr((long)val));
                            break;
                        case IntPtr ptr:
                            arguments.Add(ptr);
                            break;
                        default:
                            throw new ApplicationException($"unsupported type {arg.GetType()} {arg}");
                    }
                }

                _client.Send(new CallFunctionRequest(address, arguments));
                var message = _client.Receive() as StatusWithValueResponse;
                if (message.Success)
                {
                    return new IntPtr(BitConverter.ToInt64(message.Value, 0));
                }

                throw new ApplicationException(Encoding.ASCII.GetString(message.Value));
            }
        }

        public IntPtr CreateFunction(string name, uint numArgs, CallHandler handler)
        {
            lock (this)
            {
                if (_functions.ContainsKey(name))
                {
                    throw new ApplicationException("Function with this name already exists");
                }

                _client.Send(new CreateFunctionRequest(name, numArgs));
                var message = _client.Receive() as StatusWithValueResponse;
                if (message.Success)
                {
                    var address = new IntPtr(BitConverter.ToInt64(message.Value, 0));
                    _functions[name] = handler;
                    return address;
                }

                throw new ApplicationException(Encoding.ASCII.GetString(message.Value));
            }
        }

        public bool FreeFunction(string name)
        {
            lock (this)
            {
                if (!_functions.Remove(name))
                {
                    return false;
                }

                _client.Send(new FreeFunctionRequest(name));
                var message = _client.Receive() as StatusResponse;
                return message.Success;
            }
        }
    }

    [Flags]
    public enum MemoryProtectionConstraints : uint
    {
        PageExecute = 0x10,
        PageExecuteRead = 0x20,
        PageExecuteReadWrite = 0x40,
        PageExecuteWriteCopy = 0x80,
        PageNoAccess = 0x01,
        PageReadOnly = 0x02,
        PageReadWrite = 0x04,
        PageWriteCopy = 0x08,
        PageTargetsInvalid = 0x40000000,
        PageTargetsNoUpdate = 0x40000000,
        PageGuard = 0x100,
        PageNoCache = 0x200,
        PageWriteCombine = 0x400
    }

    [Flags]
    public enum MemoryAllocationType
    {
        MemCommit = 0x00001000,
        MemReserve = 0x00002000
    }
}