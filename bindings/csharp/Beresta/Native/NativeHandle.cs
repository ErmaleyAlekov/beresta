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
    /// The base of handles owned by the caller (document, XPath, schema).
    /// 
    /// SafeHandle is the standard .NET mechanism for unmanaged resources:
    /// release is guaranteed on Dispose, on finalization and even on an emergency
    /// domain unload; the reference count prevents releasing the resource while a
    /// P/Invoke call is in progress (protection against races).
    /// A concrete class defines only the release function (Template Method).
    /// </summary>
    internal abstract class NativeHandle : SafeHandle
    {
        protected NativeHandle(IntPtr pointer)
            : base(IntPtr.Zero, true)
        {
            SetHandle(pointer);
        }

        /// <summary>Нулевой указатель — «нет ресурса», освобождать нечего.
        /// A null pointer means "no resource", nothing to release.</summary>
        public override bool IsInvalid
        {
            get { return handle == IntPtr.Zero; }
        }
    }

    /// <summary>Дескриптор документа: beresta_document_free.
    /// The document handle: beresta_document_free.</summary>
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
