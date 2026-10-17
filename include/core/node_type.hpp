#ifndef NODE_TYPE_HPP
#define NODE_TYPE_HPP

namespace beresta
{
    /*
    ** Тип узла дерева. Позволяет безопасно определить реальный класс
    ** узла перед static_cast, не прибегая к dynamic_cast и RTTI.
    */
    enum class NodeType
    {
        DOCUMENT,               // Корневой контейнер документа (Document)
        DOCUMENT_TYPE,          // Объявление <!DOCTYPE ...> (DocumentType)
        ELEMENT,                // Элемент с тегом и атрибутами (Element)
        TEXT,                   // Текстовый узел (Text)
        CDATA_SECTION,          // Секция <![CDATA[...]]> (CDataSection)
        COMMENT,                // Комментарий <!--...--> (Comment)
        PROCESSING_INSTRUCTION  // Инструкция обработки <?target data?>
    };
}

#endif
