package org.beresta;

import org.beresta.internal.NativeStrings;

/**
 * Тип узла; порядок констант совпадает с beresta_node_kind (ordinal = код).
 * <p>Node type; the constant order matches beresta_node_kind (ordinal = code).
 */
public enum NodeType
{
    DOCUMENT,
    DOCUMENT_TYPE,
    ELEMENT,
    TEXT,
    CDATA_SECTION,
    COMMENT,
    PROCESSING_INSTRUCTION;

    private static final NodeType[] BY_CODE = values();

    /** Тип по коду из Си-интерфейса. / The type by its C interface code. */
    public static NodeType fromCode(int code)
    {
        if (code < 0 || code >= BY_CODE.length)
            throw new IllegalArgumentException("Берёста / Beresta: " + NativeStrings.message(5, code));
        return BY_CODE[code];
    }
}
