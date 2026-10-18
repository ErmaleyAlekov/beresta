#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "utils/string_utils.hpp"

namespace beresta::detail
{
    /*
    ** Комментарий <!--...-->. Ищем первое "--": по стандарту XML за ним
    ** обязан сразу идти '>', иначе это запрещённое "--" внутри комментария.
    **
    ** A comment <!--...-->. Looks for the first "--": by the XML standard it must
    ** be followed immediately by '>', otherwise it is a forbidden "--" inside the comment.
    */
    void ParseSession::parseComment()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(4);
        std::string_view content = scanner.readUntil("--");
        if (!scanner.startsWith(">"))
            scanner.failAt(i18n::format(i18n::Message::COMMENT_DOUBLE_HYPHEN_INSIDE),
                scanner.getOffset() - 2);
        scanner.advance(1);
        markEvent();
        if (_validator)
            _validator->markup();
        _handler.comment(content);
    }

    // Секция CDATA: всё до "]]>" дословно; вне корня запрещена
    // A CDATA section: everything up to "]]>" verbatim; forbidden outside the root
    void ParseSession::parseCData()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(9);
        std::string_view content = scanner.readUntil("]]>");
        if (_openElements.empty())
            scanner.failAt(i18n::format(i18n::Message::CDATA_OUTSIDE_ROOT), _tokenStart);
        markEvent();
        if (_validator)
            _validator->text(true);
        _handler.cdata(content);
    }

    /*
    ** Инструкция обработки <?target data?>. Цель "xml" здесь — это
    ** XML-декларация не на своём месте (своё место — самое начало документа).
    ** Данные — всё от первого непробельного символа до "?>", ВКЛЮЧАЯ
    ** пробелы в конце (раздел 2.6: они часть данных инструкции).
    ** С пространствами имён цель не может содержать ':' (Namespaces 1.0, разд. 7).
    **
    ** A processing instruction <?target data?>. The "xml" target here is
    ** an XML declaration in the wrong place (its place is the very start of the document).
    ** The data is everything from the first non-whitespace character up to "?>", INCLUDING
    ** the trailing spaces (section 2.6: they are part of the instruction data).
    ** With namespaces the target cannot contain ':' (Namespaces 1.0, section 7).
    */
    void ParseSession::parseProcessingInstruction()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(2);
        std::string_view target = scanner.readName();
        if (utils::equalsIgnoreCaseAscii(target, "xml"))
            scanner.failAt(i18n::format(i18n::Message::XML_DECLARATION_PLACEMENT), _tokenStart);
        if (_options.namespaceAware && target.find(':') != std::string_view::npos)
            scanner.failAt(i18n::format(i18n::Message::PI_TARGET_COLON), _tokenStart,
                DomErrorCode::NAMESPACE_ERROR);
        if (!scanner.startsWith("?>") && scanner.skipWhitespace() == 0)
            scanner.fail(i18n::format(i18n::Message::PI_SPACE_AFTER_TARGET));
        std::string_view data = scanner.readUntil("?>");
        markEvent();
        if (_validator)
            _validator->markup();
        _handler.processingInstruction(target, data);
    }

    /*
    ** Закрывающий тег </name>. Имя обязано совпасть с последним открытым
    ** элементом, а внутри сущности нельзя закрыть элемент, открытый снаружи.
    **
    ** An end tag </name>. The name must match the last open
    ** element, and inside an entity an element opened outside cannot be closed.
    */
    void ParseSession::parseEndTag()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(2);
        std::string name(scanner.readName());
        scanner.skipWhitespace();
        scanner.expect(">");
        if (_openElements.empty())
            scanner.failAt(i18n::format(i18n::Message::END_TAG_WITHOUT_START, {name}), _tokenStart);
        if (_inputs.inEntity() && _openElements.size() <= _inputs.top().openElements)
            scanner.failAt(i18n::format(i18n::Message::END_TAG_OUTSIDE_ENTITY, {name}),
                _tokenStart);
        const std::string& expected = _openElements.back().name;
        if (expected != name)
            scanner.failAt(i18n::format(i18n::Message::END_TAG_MISMATCH, {expected, name}), _tokenStart);
        emitEndElement();
    }
}
