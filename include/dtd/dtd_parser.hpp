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
    **
    ** DtdParser — recursive descent over the DTD grammar (section 3 of the XML specification).
    ** Fills DtdModel with entity, element, attribute and notation declarations.
    ** The lexis (spaces, %pe;, literals) is delegated to DtdReader, so
    ** only the declaration syntax is here.
    */
    class DtdParser
    {
    public:
        explicit DtdParser(DtdReader& reader) noexcept;

        // Внутреннее подмножество: до ']' (сама скобка не поглощается)
        // The internal subset: up to ']' (the bracket itself is not consumed)
        void parseInternalSubset();

        // Внешнее подмножество: до конца текста
        // The external subset: up to the end of the text
        void parseExternalSubset();

    private:
        // ---- Список объявлений (dtd_parser.cpp) ----
        // ---- The declaration list (dtd_parser.cpp) ----
        void parseDeclarations(bool stopAtBracket, bool stopAtSectionEnd);
        void parseMarkupDeclaration();
        bool referencesInMarkup() const noexcept;

        // ---- Сущности (dtd_parser_entities.cpp) ----
        // ---- Entities (dtd_parser_entities.cpp) ----
        void parseEntityDeclaration();
        void parseExternalEntity(EntityDeclaration& entity);
        std::string parseEntityValue();
        std::size_t appendValueReference(std::string_view literal, std::size_t index,
            std::string& value);

        // ---- Элементы (dtd_parser_elements.cpp) ----
        // ---- Elements (dtd_parser_elements.cpp) ----
        void parseElementDeclaration();
        void parseContentSpec(ElementDeclaration& declaration);
        void parseMixed(ElementDeclaration& declaration, std::size_t openFrame);
        content::Particle parseGroup(std::size_t depth, std::size_t openFrame);
        content::Particle parseContentParticle(std::size_t depth);

        // ---- Атрибуты (dtd_parser_attributes.cpp) ----
        // ---- Attributes (dtd_parser_attributes.cpp) ----
        void parseAttlistDeclaration();
        void parseAttributeDefinition(const std::string& element);
        void parseAttributeType(AttributeDeclaration& declaration);
        void parseEnumeration(AttributeDeclaration& declaration, bool names);
        void parseDefaultDeclaration(AttributeDeclaration& declaration);

        // ---- Прочее (dtd_parser_misc.cpp) ----
        // ---- Miscellaneous (dtd_parser_misc.cpp) ----
        void parseOccurrence(content::Particle& particle);
        void parseNotationDeclaration();
        void parseConditionalSection();
        void skipIgnoredSection();

        // ---- PI, комментарии и вложенность PE (dtd_parser_markup.cpp) ----
        // ---- PIs, comments and PE nesting (dtd_parser_markup.cpp) ----
        void parseProcessingInstruction();
        void parseComment();
        // Имя сущности или нотации; с пространствами имён ':' в нём запрещено
        // An entity or notation name; with namespaces ':' is forbidden in it
        std::string readDeclaredName();
        // Объявление обязано начаться и закончиться в одном тексте (WFC [28a] и VC 2.8)
        // A declaration must start and end in the same text (WFC [28a] and VC 2.8)
        void checkDeclarationNesting(std::size_t startFrame);
        // ')' группы — в том же тексте, что и '(' (VC: Proper Group/PE Nesting)
        // The ')' of a group is in the same text as the '(' (VC: Proper Group/PE Nesting)
        void checkGroupNesting(std::size_t openFrame);

        DtdReader& _reader;
    };
}

#endif
