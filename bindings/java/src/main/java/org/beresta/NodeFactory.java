package org.beresta;

import java.lang.foreign.MemorySegment;

import org.beresta.internal.NodeCalls;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NodeFunctions;

/**
 * Переходы между нативными дескрипторами и объектами Java.
 * wrap — фабрика обёрток (паттерн Factory Method): по типу узла за
 * дескриптором выбирается класс — XmlElement, сам XmlDocument или XmlNode.
 *
 * <p>Transitions between native handles and Java objects.
 * wrap is the wrapper factory (the Factory Method pattern): the class is chosen by
 * the node type behind the handle — XmlElement, the XmlDocument itself or XmlNode.
 */
final class NodeFactory
{
    private NodeFactory()
    {
    }

    /**
     * Обёртка результата Си-функции; NULL без ошибки — null. Уже выданная
     * живая обёртка того же узла возвращается повторно (реестр документа,
     * паттерн Identity Map): у одного узла одна обёртка.
     *
     * <p>Wraps the result of a C function; NULL without an error is null. A live
     * wrapper already handed out for the same node is returned again (the document
     * registry, the Identity Map pattern): one node has one wrapper.
     */
    static XmlNode wrap(Object result, XmlDocument document)
    {
        MemorySegment pointer = NativeCalls.handle(result);

        if (pointer == null)
            return null;
        if (pointer.address() == document.address())
            return document;
        boolean element = NodeCalls.type(pointer) == NodeType.ELEMENT.ordinal();
        XmlNode node = document.registry.find(pointer.address(), element);
        if (node != null)
            return node;
        if (element)
            node = new XmlElement(pointer, document);
        else
            node = new XmlNode(pointer, document);
        document.registry.store(pointer.address(), node);
        return node;
    }

    /**
     * Дескриптор схемы для разбора (null -> NULL: без проверки по схеме).
     * <p>The schema handle for parsing (null -> NULL: no schema validation).
     */
    static MemorySegment schemaHandle(XmlSchema schema)
    {
        if (schema == null)
            return MemorySegment.NULL;
        return schema.pointer;
    }
}
