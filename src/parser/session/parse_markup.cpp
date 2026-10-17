#include "parser/parse_session.hpp"
#include "utils/string_utils.hpp"

namespace beresta::detail
{
    /*
    ** Комментарий <!--...-->. Ищем первое "--": по стандарту XML за ним
    ** обязан сразу идти '>', иначе это запрещённое "--" внутри комментария.
    */
    void ParseSession::parseComment()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(4);
        std::string_view content = scanner.readUntil("--");
        if (!scanner.startsWith(">"))
            scanner.failAt("последовательность \"--\" недопустима внутри комментария",
                scanner.getOffset() - 2);
        scanner.advance(1);
        markEvent();
        _handler.comment(content);
    }

    // Секция CDATA: всё до "]]>" дословно; вне корня запрещена
    void ParseSession::parseCData()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(9);
        std::string_view content = scanner.readUntil("]]>");
        if (_openElements.empty())
            scanner.failAt("секция CDATA вне корневого элемента недопустима", _tokenStart);
        markEvent();
        if (_validator)
            _validator->text(true);
        _handler.cdata(content);
    }

    /*
    ** Инструкция обработки <?target data?>. Цель "xml" здесь — это
    ** XML-декларация не на своём месте (своё место — самое начало документа).
    ** Пробелы в конце данных отбрасываются: при выводе они были бы лишними.
    */
    void ParseSession::parseProcessingInstruction()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(2);
        std::string_view target = scanner.readName();
        if (utils::equalsIgnoreCaseAscii(target, "xml"))
            scanner.failAt("XML-декларация допустима только в самом начале документа", _tokenStart);
        if (!scanner.startsWith("?>") && scanner.skipWhitespace() == 0)
            scanner.fail("после цели инструкции обработки ожидался пробел");
        std::string_view data = utils::trimRight(scanner.readUntil("?>"));
        markEvent();
        _handler.processingInstruction(target, data);
    }

    /*
    ** Закрывающий тег </name>. Имя обязано совпасть с последним открытым
    ** элементом, а внутри сущности нельзя закрыть элемент, открытый снаружи.
    */
    void ParseSession::parseEndTag()
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(2);
        std::string name(scanner.readName());
        scanner.skipWhitespace();
        scanner.expect(">");
        if (_openElements.empty())
            scanner.failAt("закрывающий тег </" + name + "> без открывающего", _tokenStart);
        if (_inputs.inEntity() && _openElements.size() <= _inputs.top().openElements)
            scanner.failAt("тег </" + name + "> закрывает элемент, открытый вне сущности",
                _tokenStart);
        const std::string& expected = _openElements.back().name;
        if (expected != name)
            scanner.failAt("ожидался </" + expected + ">, найден </" + name + ">", _tokenStart);
        emitEndElement();
    }
}
