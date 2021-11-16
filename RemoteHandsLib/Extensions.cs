using System;
using System.Diagnostics.Contracts;
using System.IO;

namespace RemoteHands
{
    internal static class Extensions
    {
        public static IntPtr ReadIntPtr(this BinaryReader br)
        {
            Contract.Requires(br != null);
            return (IntPtr)br.ReadInt64();
        }

        public static void Write(this BinaryWriter bw, IntPtr value)
        {
            Contract.Requires(bw != null);
            bw.Write(value.ToInt64());
        }
    }
}