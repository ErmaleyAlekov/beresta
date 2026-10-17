#ifndef DTD_PARSER_HPP
#define DTD_PARSER_HPP

#include "dtd/dtd_reader.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::dtd
{
    /*
    ** DtdParser — рекурсивный спуск по грамматике DTD (раздел 3 спецификации XML).
    ** Заполняет DtdModel объявлениями сущностей, элементов, атрибутов и нотаций.
    ** Лексика (пробелы, %pe;, литералы) делегирована DtdReader, поэтому
    ** здесь — только синтаксис объявлений.
    */
    class DtdParser
    {
    public:
        explicit DtdParser(DtdReader& reader) noexcept;

        // Внутреннее подмножество: до ']' (сама скобка не поглощается)
        void parseInternalSubset();

        // Внешнее подмножество: до конца текста
        void parseExternalSubset();

    private:
        // ---- Список объявлений (dtd_parser.cpp) ----
        void parseDeclarations(bool stopAtBracket, bool stopAtSectionEnd);
        void parseMarkupDeclaration();
        bool referencesInMarkup() const noexcept;

        // ---- Сущности (dtd_parser_entities.cpp) ----
        void parseEntityDeclaration();
        void parseExternalEntity(EntityDeclaration& entity);
        std::string parseEntityValue();
        std::size_t appendValueReference(std::string_view literal, std::size_t index,
            std::string& value);

        // ---- Элементы (dtd_parser_elements.cpp) ----
        void parseElementDeclaration();
        void parseContentSpec(ElementDeclaration& declaration);
        void parseMixed(ElementDeclaration& declaration);
        content::Particle parseGroup(std::size_t depth);
        content::Particle parseContentParticle(std::size_t depth);

        // ---- Атрибуты (dtd_parser_attributes.cpp) ----
        void parseAttlistDeclaration();
        void parseAttributeDefinition(const std::string& element);
        void parseAttributeType(AttributeDeclaration& declaration);
        void parseEnumeration(AttributeDeclaration& declaration, bool names);
        void parseDefaultDeclaration(AttributeDeclaration& declaration);

        // ---- Прочее (dtd_parser_misc.cpp) ----
        void parseOccurrence(content::Particle& particle);
        void parseNotationDeclaration();
        void parseConditionalSection();
        void skipIgnoredSection();

        DtdReader& _reader;
    };
}

#endif
