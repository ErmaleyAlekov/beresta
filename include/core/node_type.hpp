#ifndef NODE_TYPE_HPP
#define NODE_TYPE_HPP

namespace beresta
{
    /*
    ** Тип узла дерева. Позволяет безопасно определить реальный класс
    ** узла перед static_cast, не прибегая к dynamic_cast и RTTI.
    **
    ** The tree node type. Allows determining the real class of the node
    ** safely before a static_cast, without resorting to dynamic_cast and RTTI.
    */
    enum class NodeType
    {
        DOCUMENT,               // Корневой контейнер документа (Document)
                                // The root container of the document (Document)
        DOCUMENT_TYPE,          // Объявление <!DOCTYPE ...> (DocumentType)
                                // The <!DOCTYPE ...> declaration (DocumentType)
        ELEMENT,                // Элемент с тегом и атрибутами (Element)
                                // An element with a tag and attributes (Element)
        TEXT,                   // Текстовый узел (Text) / A text node (Text)
        CDATA_SECTION,          // Секция <![CDATA[...]]> (CDataSection) / A <![CDATA[...]]> section (CDataSection)
        COMMENT,                // Комментарий <!--...--> (Comment) / A <!--...--> comment (Comment)
        PROCESSING_INSTRUCTION  // Инструкция обработки <?target data?> / A <?target data?> processing instruction
    };
}

#endif
