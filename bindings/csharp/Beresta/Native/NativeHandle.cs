using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>
    /// Основа дескрипторов, которыми владеет вызывающий (документ, XPath, схема).
    ///
    /// SafeHandle — стандартный механизм .NET для неуправляемых ресурсов:
    /// освобождение гарантировано и при Dispose, и при финализации, и даже
    /// при аварийной выгрузке домена; счётчик ссылок не даёт освободить
    /// ресурс во время идущего P/Invoke-вызова (защита от гонок).
    /// Конкретный класс определяет только функцию освобождения (Template Method).
    /// </summary>
    internal abstract class NativeHandle : SafeHandle
    {
        protected NativeHandle(IntPtr pointer)
            : base(IntPtr.Zero, true)
        {
            SetHandle(pointer);
        }

        /// <summary>Нулевой указатель — «нет ресурса», освобождать нечего.</summary>
        public override bool IsInvalid
        {
            get { return handle == IntPtr.Zero; }
        }
    }

    /// <summary>Дескриптор документа: beresta_document_free.</summary>
    internal sealed class DocumentHandle : NativeHandle
    {
        internal DocumentHandle(IntPtr pointer)
            : base(pointer)
        {
        }

        protected override bool ReleaseHandle()
        {
            NativeMethods.beresta_document_free(handle);
            return true;
        }
    }
}
