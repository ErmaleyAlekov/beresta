package org.beresta;

import java.lang.foreign.MemorySegment;

/**
 * Основа всех интерфейсов узла.
 *
 * <p>Методы узла разнесены по интерфейсам с методами по умолчанию
 * (NodeContent, NodeNavigation, NodeChildren, NodeEditing, NodeStructure):
 * каждый отвечает за одну обязанность — принцип разделения интерфейсов
 * (ISP из SOLID). Класс XmlNode лишь собирает их вместе, а пользователь
 * может принимать в своих методах ровно тот интерфейс, который ему нужен.
 *
 * <p>The base of all node interfaces.
 * Node methods are split into interfaces with default methods
 * (NodeContent, NodeNavigation, NodeChildren, NodeEditing, NodeStructure):
 * each has one responsibility — the interface segregation principle
 * (ISP from SOLID). The XmlNode class only combines them, and a user
 * can accept in their methods exactly the interface they need.
 */
public interface NodeBase
{
    /**
     * Нативный дескриптор узла; бросает IllegalStateException, если документ закрыт.
     * <p>The native node handle; throws IllegalStateException if the document is closed.
     */
    MemorySegment handle();

    /** Документ, которому принадлежит узел. / The document the node belongs to. */
    XmlDocument document();
}
