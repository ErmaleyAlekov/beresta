#ifndef NODE_FILTER_HPP
#define NODE_FILTER_HPP

#include "core/node.hpp"

namespace beresta
{
    // Решение фильтра (DOM Level 2 Traversal)
    enum class FilterResult
    {
        ACCEPT,     // узел возвращается обходом
        SKIP,       // узел пропускается, но его дети рассматриваются
        REJECT      // узел пропускается вместе со всем поддеревом (для NodeIterator = SKIP)
    };

    /*
    ** Маски типов узлов для whatToShow: какие узлы вообще видит обход.
    ** Комбинируются побитовым ИЛИ: show::ELEMENTS | show::TEXTS.
    */
    namespace show
    {
        inline constexpr unsigned ALL_NODES = 0xFFFFFFFFu;
        inline constexpr unsigned DOCUMENTS = 1u << 0;
        inline constexpr unsigned DOCUMENT_TYPES = 1u << 1;
        inline constexpr unsigned ELEMENTS = 1u << 2;
        inline constexpr unsigned TEXTS = 1u << 3;
        inline constexpr unsigned CDATA_SECTIONS = 1u << 4;
        inline constexpr unsigned COMMENTS = 1u << 5;
        inline constexpr unsigned PROCESSING_INSTRUCTIONS = 1u << 6;
    }

    // Бит маски whatToShow для типа узла
    unsigned showMaskFor(NodeType type) noexcept;

    /*
    ** NodeFilter — пользовательский фильтр обхода (паттерн Strategy).
    ** Реализуется классом с методом acceptNode — без лямбд и колбэков.
    */
    class NodeFilter
    {
    public:
        virtual ~NodeFilter();

        virtual FilterResult acceptNode(const Node& node) const = 0;
    };
}

#endif
