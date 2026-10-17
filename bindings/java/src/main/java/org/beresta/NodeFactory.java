package org.beresta;

import java.lang.foreign.MemorySegment;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NodeFunctions;

/**
 * Переходы между нативными дескрипторами и объектами Java.
 * wrap — фабрика обёрток (паттерн Factory Method): по типу узла за
 * дескриптором выбирается класс — XmlElement, сам XmlDocument или XmlNode.
 */
final class NodeFactory
{
    private NodeFactory()
    {
    }

    /** Обёртка результата Си-функции; NULL без ошибки — null. */
    static XmlNode wrap(Object result, XmlDocument document)
    {
        MemorySegment pointer = NativeCalls.handle(result);

        if (pointer == null)
            return null;
        if (pointer.address() == document.address())
            return document;
        int type = (int) NativeCalls.invoke(NodeFunctions.TYPE, pointer);
        if (type == NodeType.ELEMENT.ordinal())
            return new XmlElement(pointer, document);
        return new XmlNode(pointer, document);
    }

    /** Дескриптор схемы для разбора (null -> NULL: без проверки по схеме). */
    static MemorySegment schemaHandle(XmlSchema schema)
    {
        if (schema == null)
            return MemorySegment.NULL;
        return schema.pointer;
    }
}
