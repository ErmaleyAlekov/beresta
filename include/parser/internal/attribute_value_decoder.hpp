#ifndef ATTRIBUTE_VALUE_DECODER_HPP
#define ATTRIBUTE_VALUE_DECODER_HPP

#include "dtd/dtd_model.hpp"
#include "parser/internal/expansion_budget.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::detail
{
    /*
    ** AttributeValueDecoder — нормализация значения атрибута по разделу 3.3.3
    ** спецификации XML: символьные ссылки раскрываются как есть, литеральные
    ** \t \n \r превращаются в пробел (\r\n — в один пробел), общие сущности
    ** раскрываются рекурсивно с проверкой бюджета и циклов, '<' запрещён.
    ** Используется и для атрибутов в тегах, и для значений по умолчанию в DTD.
    ** Ошибки — DomException (позицию добавляет вызывающий).
    **
    ** AttributeValueDecoder — attribute value normalization by section 3.3.3
    ** of the XML specification: character references are expanded as is, literal
    ** \t \n \r become a space (\r\n — one space), general entities are
    ** expanded recursively with budget and cycle checks, '<' is forbidden.
    ** Used both for attributes in tags and for default values in a DTD.
    ** Errors are DomException (the caller adds the position).
    */
    class AttributeValueDecoder
    {
    public:
        // standalone — документ объявил standalone="yes": внешне объявленные сущности запрещены
        // standalone — the document declared standalone="yes": externally declared entities are forbidden
        AttributeValueDecoder(const dtd::DtdModel* model, ExpansionBudget& budget,
            bool standalone = false) noexcept;

        std::string decode(std::string_view raw) const;

    private:
        // Состояние рекурсивного раскрытия: результат и стек раскрываемых сущностей
        // The state of recursive expansion: the result and the stack of entities being expanded
        struct Expansion
        {
            std::string& output;
            std::vector<std::string_view>& active;
        };

        // Обрабатывает текст (литерал или текст замены сущности)
        // Processes a text (a literal or an entity replacement text)
        void expandText(std::string_view text, Expansion& expansion) const;

        // Обрабатывает ссылку, начинающуюся в позиции index; возвращает позицию после ';'
        // Processes a reference starting at position index; returns the position after ';'
        std::size_t expandReference(std::string_view text, std::size_t index,
            Expansion& expansion) const;

        // Раскрывает объявленную общую сущность
        // Expands a declared general entity
        void expandEntity(std::string_view name, Expansion& expansion) const;

        const dtd::DtdModel* _model;
        ExpansionBudget& _budget;
        bool _standalone;
    };
}

#endif
