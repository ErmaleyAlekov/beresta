package org.beresta;

import java.lang.foreign.MemorySegment;
import java.util.LinkedHashMap;
import java.util.Map;

import org.beresta.internal.ElementFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;

/** Элемент: узел с именем, атрибутами и пространством имён. */
public final class XmlElement extends XmlNode implements ElementAttributes
{
    XmlElement(MemorySegment pointer, XmlDocument owner)
    {
        super(pointer, owner);
    }

    /** Имя без префикса: у {@code <s:item>} — "item". */
    public String localName()
    {
        return NativeStrings.take(NativeCalls.invoke(ElementFunctions.LOCAL_NAME, handle()));
    }

    /** URI пространства имён или null, если элемент вне пространств. */
    public String namespaceUri()
    {
        return NativeStrings.take(NativeCalls.invoke(ElementFunctions.NAMESPACE_URI, handle()));
    }

    /** Снимок атрибутов {имя: значение} в порядке документа. */
    public Map<String, String> attributes()
    {
        Map<String, String> result = new LinkedHashMap<>();
        long count = (long) NativeCalls.invoke(ElementFunctions.ATTRIBUTE_COUNT, handle());

        for (long index = 0; index < count; ++index)
        {
            String name = NativeStrings.take(NativeCalls.invoke(ElementFunctions.ATTRIBUTE_NAME, handle(), index));
            result.put(name, NativeStrings.take(NativeCalls.invoke(ElementFunctions.ATTRIBUTE_VALUE, handle(), index)));
        }
        return result;
    }

    @Override
    public String toString()
    {
        return "<" + name() + ">";
    }
}
