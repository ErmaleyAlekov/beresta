package org.beresta;

/**
 * Одно нарушение схемы: текст начинается с пути к элементу
 * ("/catalog/book[2]: ..."), line и column — место в исходнике (0 — неизвестно).
 */
public record Diagnostic(String message, long line, long column)
{
}
