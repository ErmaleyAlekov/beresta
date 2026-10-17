using System.Runtime.InteropServices;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Сведения о нативной библиотеке.</summary>
    public static class BerestaLibrary
    {
        /// <summary>Версия нативной библиотеки, например "2.1.0".</summary>
        public static string Version
        {
            get { return Marshal.PtrToStringUTF8(NativeMethods.beresta_version()) ?? string.Empty; }
        }
    }
}
