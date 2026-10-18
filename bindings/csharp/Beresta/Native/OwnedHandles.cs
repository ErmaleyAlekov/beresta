using System;

namespace Beresta.Native
{
    /// <summary>Дескриптор скомпилированного выражения: beresta_xpath_free.
    /// The handle of a compiled expression: beresta_xpath_free.</summary>
    internal sealed class XPathHandle : NativeHandle
    {
        internal XPathHandle(IntPtr pointer)
            : base(pointer)
        {
        }

        protected override bool ReleaseHandle()
        {
            NativeMethods.beresta_xpath_free(handle);
            return true;
        }
    }

    /// <summary>Дескриптор схемы: beresta_schema_free.
    /// The schema handle: beresta_schema_free.</summary>
    internal sealed class SchemaHandle : NativeHandle
    {
        internal SchemaHandle(IntPtr pointer)
            : base(pointer)
        {
        }

        protected override bool ReleaseHandle()
        {
            NativeMethods.beresta_schema_free(handle);
            return true;
        }
    }
}
