#include "dtd/dtd_parser.hpp"
#include "i18n/i18n.hpp"
#include "utils/string_utils.hpp"

namespace beresta::dtd
{
    /*
    ** Инструкция обработки внутри DTD. Раньше её текст пропускался целиком,
    ** теперь цель проверяется так же, как в теле документа: это имя,
    ** не "xml" (текстовая декларация не на своём месте — ошибка разделов
    ** 2.8 и 4.3.1), без ':' при пространствах имён и с пробелом перед данными.
    ** Затем PI передаётся приложению (раздел 2.6) через dtdProcessingInstruction.
    **
    ** A processing instruction inside a DTD. Previously its text was skipped whole,
    ** now the target is checked the same way as in the document body: it is a name,
    ** not "xml" (a text declaration in the wrong place is an error of sections
    ** 2.8 and 4.3.1), without ':' with namespaces and with a space before the data.
    ** Then the PI is passed to the application (section 2.6) through dtdProcessingInstruction.
    */
    void DtdParser::parseProcessingInstruction()
    {
        detail::XmlScanner& scanner = _reader.current();

        scanner.advance(2);
        std::string_view target = scanner.readName();
        if (utils::equalsIgnoreCaseAscii(target, "xml"))
            scanner.fail(i18n::format(i18n::Message::DTD_DECLARATION_IN_DTD));
        if (_reader.getContext().options.namespaceAware && target.find(':') != std::string_view::npos)
            scanner.fail(i18n::format(i18n::Message::PI_TARGET_COLON));
        if (!scanner.startsWith("?>") && scanner.skipWhitespace() == 0)
            scanner.fail(i18n::format(i18n::Message::PI_SPACE_AFTER_TARGET));
        std::string_view data = scanner.readUntil("?>");
        ContentHandler* handler = _reader.getContext().handler;

        if (handler != nullptr)
            handler->dtdProcessingInstruction(target, data);
    }

    // Комментарий в DTD подчиняется тем же правилам: "--" внутри запрещено
    // A comment in a DTD obeys the same rules: "--" inside is forbidden
    void DtdParser::parseComment()
    {
        detail::XmlScanner& scanner = _reader.current();

        scanner.advance(4);
        scanner.readUntil("--");
        if (!scanner.startsWith(">"))
            scanner.fail(i18n::format(i18n::Message::COMMENT_DOUBLE_HYPHEN_INSIDE));
        scanner.advance(1);
    }

    // Namespaces 1.0, раздел 7: имена сущностей и нотаций не содержат ':'
    // Namespaces 1.0, section 7: entity and notation names contain no ':'
    std::string DtdParser::readDeclaredName()
    {
        std::string name = _reader.readName();

        if (_reader.getContext().options.namespaceAware && name.find(':') != std::string::npos)
            _reader.fail(i18n::format(i18n::Message::DTD_NAME_COLON, {name}));
        return name;
    }

    /*
    ** Если объявление началось внутри PE, а закончилось после её конца —
    ** это нарушение well-formedness (PE Between Declarations, продукция [28a]):
    ** PE между объявлениями обязана содержать объявления целиком. Если же
    ** конец объявления попал во вложенную PE — нарушение валидности
    ** (Proper Declaration/PE Nesting): оно копится в модели до проверки.
    **
    ** If a declaration started inside a PE and ended after its end, it is
    ** a well-formedness violation (PE Between Declarations, production [28a]):
    ** a PE between declarations must contain whole declarations. If instead
    ** the end of the declaration fell into a nested PE, it is a validity violation
    ** (Proper Declaration/PE Nesting): it is accumulated in the model until the check.
    */
    void DtdParser::checkDeclarationNesting(std::size_t startFrame)
    {
        if (_reader.frameId() == startFrame)
            return;
        if (!_reader.isFrameOpen(startFrame))
            _reader.fail(i18n::format(i18n::Message::DTD_DECLARATION_LEAVES_PE));
        _reader.getContext().model.addValidityNote(
            i18n::format(i18n::Message::DTD_DECLARATION_SPLIT_PE));
    }

    // Вызывается перед поглощением ')': кадр ')' сравнивается с кадром '('
    // Called before consuming ')': the frame of ')' is compared with the frame of '('
    void DtdParser::checkGroupNesting(std::size_t openFrame)
    {
        if (_reader.frameId() != openFrame)
            _reader.getContext().model.addValidityNote(
                i18n::format(i18n::Message::DTD_GROUP_SPLIT_PE));
    }
}
