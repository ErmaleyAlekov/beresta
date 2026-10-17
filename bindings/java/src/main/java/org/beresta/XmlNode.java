package org.beresta;

import java.lang.foreign.MemorySegment;

/**
 * Узел документа: нативный дескриптор плюс документ-владелец.
 *
 * <p>Ссылка на документ решает две задачи: пока жив узел, документ не
 * будет собран сборщиком мусора, а после {@code document.close()} любое
 * обращение к узлу даёт понятное исключение вместо падения JVM.
 */
public class XmlNode implements NodeContent, NodeOutput, NodeNavigation, NodeChildren, NodeEditing, NodeStructure
{
    private final MemorySegment pointer;
    private final XmlDocument owner;

    XmlNode(MemorySegment pointer, XmlDocument owner)
    {
        this.pointer = pointer;
        this.owner = owner;
    }

    @Override
    public MemorySegment handle()
    {
        if (document().isClosed())
            throw new IllegalStateException("Берёста: документ уже закрыт, его узлы недействительны");
        return pointer;
    }

    /** У документа владелец — он сам (owner == null в конструкторе документа). */
    @Override
    public XmlDocument document()
    {
        if (owner == null)
            return (XmlDocument) this;
        return owner;
    }

    /** Две обёртки равны, если указывают на один и тот же нативный узел. */
    @Override
    public boolean equals(Object other)
    {
        return other instanceof XmlNode node && node.pointer.address() == pointer.address();
    }

    @Override
    public int hashCode()
    {
        return Long.hashCode(pointer.address());
    }
}
