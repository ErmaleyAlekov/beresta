package org.beresta;

import java.lang.foreign.Arena;
import java.lang.invoke.MethodHandle;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NodeFunctions;

/**
 * Добавление новых узлов в конец детей. Каждый метод возвращает созданный
 * узел — удобно строить дерево: {@code root.appendElement("item").appendText("Чай")}.
 */
public interface NodeEditing extends NodeBase
{
    /** Новый элемент; у пустого документа он становится корнем. */
    default XmlElement appendElement(String name)
    {
        return (XmlElement) appendWithText(NodeFunctions.APPEND_ELEMENT, name);
    }

    default XmlNode appendText(String text)
    {
        return appendWithText(NodeFunctions.APPEND_TEXT, text);
    }

    default XmlNode appendComment(String text)
    {
        return appendWithText(NodeFunctions.APPEND_COMMENT, text);
    }

    default XmlNode appendCData(String text)
    {
        return appendWithText(NodeFunctions.APPEND_CDATA, text);
    }

    /** Общий шаг всех append_*: строка в нативной памяти, вызов, обёртка результата. */
    private XmlNode appendWithText(MethodHandle function, String text)
    {
        try (Arena arena = Arena.ofConfined())
        {
            return NodeFactory.wrap(NativeCalls.invoke(function, handle(), NativeMemory.text(arena, text)), document());
        }
    }
}
