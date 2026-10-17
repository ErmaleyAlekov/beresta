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
    */
    class AttributeValueDecoder
    {
    public:
        AttributeValueDecoder(const dtd::DtdModel* model, ExpansionBudget& budget) noexcept;

        std::string decode(std::string_view raw) const;

    private:
        // Состояние рекурсивного раскрытия: результат и стек раскрываемых сущностей
        struct Expansion
        {
            std::string& output;
            std::vector<std::string_view>& active;
        };

        // Обрабатывает текст (литерал или текст замены сущности)
        void expandText(std::string_view text, Expansion& expansion) const;

        // Обрабатывает ссылку, начинающуюся в позиции index; возвращает позицию после ';'
        std::size_t expandReference(std::string_view text, std::size_t index,
            Expansion& expansion) const;

        // Раскрывает объявленную общую сущность
        void expandEntity(std::string_view name, Expansion& expansion) const;

        const dtd::DtdModel* _model;
        ExpansionBudget& _budget;
    };
}

#endif
