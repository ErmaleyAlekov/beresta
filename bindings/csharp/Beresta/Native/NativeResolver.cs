#if NET5_0_OR_GREATER
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>
    /// Поиск нативной библиотеки для P/Invoke.
    ///
    /// Из NuGet-пакета .NET сам берёт runtimes/&lt;rid&gt;/native/. Но при ссылке
    /// на проект (и в самопроверке) файлы лежат в выходном каталоге в той же
    /// раскладке, и стандартный поиск их не видит. Этот резолвер сначала
    /// пробует стандартный поиск, затем runtimes/&lt;rid&gt;/native/ рядом со сборкой,
    /// затем переменную окружения BERESTA_LIBRARY (явный путь к файлу).
    /// Finding the native library for P/Invoke.
    /// 
    /// From a NuGet package .NET takes runtimes/&lt;rid&gt;/native/ itself. But with a
    /// project reference (and in the self-test) the files lie in the output directory
    /// in the same layout, and the standard lookup does not see them. This resolver
    /// first tries the standard lookup, then runtimes/&lt;rid&gt;/native/ next to the
    /// assembly, then the BERESTA_LIBRARY environment variable (an explicit file path).
    /// </summary>
    internal static class NativeResolver
    {
        /// <summary>Регистрация (вызывается статическим конструктором NativeMethods).
        /// Registration (called by the static constructor of NativeMethods).</summary>
        internal static void Register()
        {
            NativeLibrary.SetDllImportResolver(typeof(NativeResolver).Assembly, Resolve);
        }

        private static IntPtr Resolve(string name, Assembly assembly, DllImportSearchPath? searchPath)
        {
            if (name != NativeMethods.Library)
                return IntPtr.Zero;
            string? explicitPath = Environment.GetEnvironmentVariable("BERESTA_LIBRARY");
            if (!string.IsNullOrEmpty(explicitPath))
                return NativeLibrary.Load(explicitPath);
            if (NativeLibrary.TryLoad(name, assembly, searchPath, out IntPtr loaded))
                return loaded;
            string bundled = Path.Combine(AppContext.BaseDirectory, "runtimes", RuntimeId(), "native", FileName());
            if (File.Exists(bundled))
                return NativeLibrary.Load(bundled);
            return IntPtr.Zero;
        }

        /// <summary>Идентификатор среды: win-x64, linux-arm64, osx-arm64...
        /// The runtime identifier: win-x64, linux-arm64, osx-arm64...</summary>
        internal static string RuntimeId()
        {
            string system = "linux";
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                system = "win";
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                system = "osx";
            string arch = RuntimeInformation.ProcessArchitecture.ToString().ToLowerInvariant();
            return system + "-" + arch;
        }

        /// <summary>Каноническое имя файла библиотеки на текущей ОС.
        /// The canonical library file name on the current OS.</summary>
        private static string FileName()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                return "beresta.dll";
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                return "libberesta.dylib";
            return "libberesta.so";
        }
    }
}
#endif
