package org.beresta;

/** Тип узла; порядок констант совпадает с beresta_node_kind (ordinal = код). */
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

    /** Тип по коду из Си-интерфейса. */
    public static NodeType fromCode(int code)
    {
        if (code < 0 || code >= BY_CODE.length)
            throw new IllegalArgumentException("Берёста: неизвестный тип узла " + code);
        return BY_CODE[code];
    }
}
