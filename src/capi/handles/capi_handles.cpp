#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"

namespace beresta::capi
{
    /*
    ** Все дескрипторы документов и узлов хранят указатель Node*,
    ** приведённый к void*. Обратное приведение static_cast<Node*>(void*)
    ** поэтому всегда корректно, а конкретный тип узнаётся через
    ** dynamic_cast — безопасно, без доверия к внешнему коду.
    **
    ** All document and node handles keep a Node* pointer cast to void*.
    ** The reverse cast static_cast<Node*>(void*) is therefore always correct,
    ** and the concrete type is determined through dynamic_cast — safely,
    ** without trusting external code.
    */
    Node& toNode(void* handle)
    {
        if (handle == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::HANDLE_EXPECTED_NODE));
        return *static_cast<Node*>(handle);
    }

    // Элемент: любой другой узел (текст, документ...) — ошибка аргумента
    // An element: any other node (text, document...) is an argument error
    Element& toElement(void* handle)
    {
        Element* element = dynamic_cast<Element*>(&toNode(handle));

        if (element == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::HANDLE_EXPECTED_ELEMENT));
        return *element;
    }

    // Документ: дескриптор от beresta_document_* или от owner_document
    // A document: a handle from beresta_document_* or from owner_document
    Document& toDocument(void* handle)
    {
        Document* document = dynamic_cast<Document*>(&toNode(handle));

        if (document == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::HANDLE_EXPECTED_DOCUMENT));
        return *document;
    }

    // Узел с детьми: элемент или документ
    // A node with children: an element or a document
    ContainerNode& toContainer(void* handle)
    {
        ContainerNode* container = dynamic_cast<ContainerNode*>(&toNode(handle));

        if (container == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::HANDLE_EXPECTED_CONTAINER));
        return *container;
    }

    /*
    ** Индекс приходит из внешнего мира как int64_t: отрицательное значение
    ** или выход за размер — ошибка аргумента, а не неопределённое поведение.
    **
    ** The index comes from the outside world as int64_t: a negative value
    ** or exceeding the size is an argument error, not undefined behavior.
    */
    std::size_t checkedIndex(std::int64_t index, std::size_t size)
    {
        if (index < 0 || static_cast<std::uint64_t>(index) >= size)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::INDEX_OUT_OF_BOUNDS, {std::to_string(index), std::to_string(size)}));
        return static_cast<std::size_t>(index);
    }
}
