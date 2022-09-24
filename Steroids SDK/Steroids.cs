using System.Runtime.InteropServices;

namespace Steroids
{
    public static partial class Steroids
    {
        [LibraryImport("SteroidsCore.dll", EntryPoint = "RequestSteroids", SetLastError = true)]
        [UnmanagedCallConv(CallConvs = new Type[] { typeof(System.Runtime.CompilerServices.CallConvFastcall) })]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static partial bool RequestSteroids();
    }
}