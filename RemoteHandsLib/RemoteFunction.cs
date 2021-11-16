using System;
using System.Collections.Generic;

namespace RemoteHands
{
    public enum ResponseFlags
    {
        None = 0,
        PrintStack = 1
    }

    public class CallResponse
    {
        public ResponseFlags Falgs;
        public IntPtr ReturnValue;

        public static implicit operator CallResponse(IntPtr ptr)
        {
            return new CallResponse { ReturnValue = ptr };
        }
    }

    public delegate CallResponse CallHandler(List<IntPtr> arguments);
}