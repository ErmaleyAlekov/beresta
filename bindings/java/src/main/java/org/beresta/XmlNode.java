package org.beresta;

import java.lang.foreign.MemorySegment;

import org.beresta.internal.NativeStrings;

/**
 * Узел документа: нативный дескриптор плюс документ-владелец.
 *
 * <p>Ссылка на документ решает две задачи: пока жив узел, документ не
 * будет собран сборщиком мусора, а после {@code document.close()} любое
 * обращение к узлу даёт понятное исключение вместо падения JVM.
 *
 * <p>A document node: a native handle plus the owning document.
 * The document reference solves two problems: while the node is alive, the
 * document is not garbage-collected, and after {@code document.close()} any
 * access to the node gives a clear exception instead of a JVM crash.
 */
public class XmlNode implements NodeContent, NodeOutput, NodeNavigation, NodeChildren, NodeEditing, NodeStructure
{
    private final MemorySegment pointer;

    // Не final и доступен пакету: NodeRegistry меняет владельца при moveTo
    // Not final and package-visible: NodeRegistry changes the owner on moveTo
    XmlDocument owner;

    XmlNode(MemorySegment pointer, XmlDocument owner)
    {
        this.pointer = pointer;
        this.owner = owner;
    }

    @Override
    public MemorySegment handle()
    {
        if (document().isClosed())
            throw new IllegalStateException("Берёста / Beresta: " + NativeStrings.message(1));
        return pointer;
    }

    /**
     * У документа владелец — он сам (owner == null в конструкторе документа).
     * <p>A document's owner is the document itself (owner == null in the document constructor).
     */
    @Override
    public XmlDocument document()
    {
        if (owner == null)
            return (XmlDocument) this;
        return owner;
    }

    /**
     * Две обёртки равны, если указывают на один и тот же нативный узел.
     * <p>Two wrappers are equal if they point to the same native node.
     */
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
