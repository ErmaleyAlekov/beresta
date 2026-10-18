package org.beresta;

/**
 * Одно нарушение схемы: текст начинается с пути к элементу
 * ("/catalog/book[2]: ..."), line и column — место в исходнике (0 — неизвестно).
 *
 * <p>One schema violation: the text starts with the element path
 * ("/catalog/book[2]: ..."), line and column are the place in the source (0 — unknown).
 */
public record Diagnostic(String message, long line, long column)
{
}
