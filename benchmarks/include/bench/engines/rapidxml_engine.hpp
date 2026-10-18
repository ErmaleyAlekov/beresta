#ifndef BERESTA_BENCH_RAPIDXML_ENGINE_HPP
#define BERESTA_BENCH_RAPIDXML_ENGINE_HPP

#include "bench/engine.hpp"

#include <rapidxml/rapidxml.hpp>

#include <vector>

namespace bench
{
    /*
    ** RapidXML — разбор «на месте» (in-situ): парсер пишет нули прямо во
    ** входной буфер и не копирует строки. Это предел скорости, но ценой
    ** строгости: кодировки, пространства имён и многие ошибки синтаксиса
    ** не проверяются. Вход копируется в изменяемый буфер — копия входит
    ** в замер, без неё разбор невозможен.
    **
    ** RapidXML — in-situ parsing: the parser writes zeros right into the input
    ** buffer and does not copy strings. This is the speed limit, but at the cost of
    ** strictness: encodings, namespaces and many syntax errors are not checked.
    ** The input is copied into a mutable buffer — the copy is part of the
    ** measurement, parsing is impossible without it.
    */
    class RapidxmlEngine : public XmlEngine
    {
    public:
        std::string name() const override;
        void parse(const std::string& xml) override;
        std::size_t traverse() const override;
        void release() override;
        std::optional<std::size_t> serialize() const override;

    private:
        std::vector<char> _buffer;
        rapidxml::xml_document<> _document;
    };
}

#endif
