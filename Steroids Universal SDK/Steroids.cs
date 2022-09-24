using System.Runtime.InteropServices;

namespace Steroids
{
    public static class Steroids
    {
        [DllImport("SteroidsCore.dll", EntryPoint = "RequestSteroids", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool RequestSteroids();
    }
}
