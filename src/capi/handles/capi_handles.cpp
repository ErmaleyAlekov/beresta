#include "capi/internal/capi_support.hpp"

namespace beresta::capi
{
    /*
    ** Все дескрипторы документов и узлов хранят указатель Node*,
    ** приведённый к void*. Обратное приведение static_cast<Node*>(void*)
    ** поэтому всегда корректно, а конкретный тип узнаётся через
    ** dynamic_cast — безопасно, без доверия к внешнему коду.
    */
    Node& toNode(void* handle)
    {
        if (handle == nullptr)
            throw InvalidArgumentException("ожидался узел, передан NULL");
        return *static_cast<Node*>(handle);
    }

    // Элемент: любой другой узел (текст, документ...) — ошибка аргумента
    Element& toElement(void* handle)
    {
        Element* element = dynamic_cast<Element*>(&toNode(handle));

        if (element == nullptr)
            throw InvalidArgumentException("ожидался элемент, передан узел другого типа");
        return *element;
    }

    // Документ: дескриптор от beresta_document_* или от owner_document
    Document& toDocument(void* handle)
    {
        Document* document = dynamic_cast<Document*>(&toNode(handle));

        if (document == nullptr)
            throw InvalidArgumentException("ожидался документ, передан узел другого типа");
        return *document;
    }

    // Узел с детьми: элемент или документ
    ContainerNode& toContainer(void* handle)
    {
        ContainerNode* container = dynamic_cast<ContainerNode*>(&toNode(handle));

        if (container == nullptr)
            throw InvalidArgumentException("ожидался элемент или документ");
        return *container;
    }

    /*
    ** Индекс приходит из внешнего мира как int64_t: отрицательное значение
    ** или выход за размер — ошибка аргумента, а не неопределённое поведение.
    */
    std::size_t checkedIndex(std::int64_t index, std::size_t size)
    {
        if (index < 0 || static_cast<std::uint64_t>(index) >= size)
            throw InvalidArgumentException("индекс " + std::to_string(index)
                + " вне диапазона [0, " + std::to_string(size) + ")");
        return static_cast<std::size_t>(index);
    }
}
