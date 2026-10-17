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
 */
public interface NodeBase
{
    /** Нативный дескриптор узла; бросает IllegalStateException, если документ закрыт. */
    MemorySegment handle();

    /** Документ, которому принадлежит узел. */
    XmlDocument document();
}
