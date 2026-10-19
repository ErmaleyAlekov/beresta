package org.beresta;

/**
 * Флаги разбора (BERESTA_PARSE_*), объединяются через {@code |}:
 * {@code Xml.parse(text, ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD, null)}.
 *
 * <p>Parsing flags (BERESTA_PARSE_*), combined with {@code |}:
 * {@code Xml.parse(text, ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD, null)}.
 */
public final class ParseFlags
{
    public static final int DEFAULT = 0;
    public static final int PRESERVE_WHITESPACE = 1;
    public static final int DISCARD_COMMENTS = 1 << 1;
    public static final int COALESCE_CDATA = 1 << 2;
    public static final int VALIDATE_DTD = 1 << 3;
    public static final int NO_NAMESPACES = 1 << 4;
    public static final int XINCLUDE = 1 << 5;
    public static final int FILE_RESOURCES = 1 << 6;
    /** Строка и столбец узлов: места нарушений в validate(document). / The line and column of nodes: the places of violations in validate(document). */
    public static final int TRACK_LOCATIONS = 1 << 7;

    private ParseFlags()
    {
    }
}
