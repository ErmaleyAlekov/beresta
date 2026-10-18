#include "fuzz/fuzz_input.hpp"

#include "beresta.hpp"

#include <cstdio>
#include <cstdlib>

/*
** Цель фаззинга: DOM- и SAX-разбор с любым сочетанием настроек.
**
** Формат входа: [байт флагов][документ] 0xF5 0xF5 [внешний ресурс].
** Внешний ресурс отдаётся резолвером на любой запрос — внешний DTD,
** внешнюю сущность или XInclude, — так что фаззер мутирует и их.
**
** Оракулы (кроме самих санитайзеров ASan/UBSan):
**  1. наружу может вылететь только исключение библиотеки (DomException);
**  2. принятый документ после сериализации обязан разбираться снова
**     с теми же настройками (без проверки DTD и XInclude) — иначе
**     сериализатор порождает некорректный XML.
**
** The fuzzing target: DOM and SAX parsing with any combination of settings.
** The input format: [flag byte][document] 0xF5 0xF5 [external resource].
** The resolver returns the external resource for any request — an external DTD,
** an external entity or XInclude — so the fuzzer mutates them too.
** The oracles (besides the ASan/UBSan sanitizers themselves):
** 1. only a library exception (DomException) may escape;
** 2. an accepted document after serialization must parse again
**    with the same settings (without DTD validation and XInclude) — otherwise
**    the serializer produces invalid XML.
*/
namespace
{
    // Повторный разбор сериализованного документа; провал — ошибка сериализатора
    // Parses the serialized document again; a failure is a serializer error
    void checkRoundTrip(const beresta::Document& document, beresta::ParserOptions options)
    {
        std::string text = beresta::XmlSerializer().toString(document);

        options.validateDtd = false;
        options.processXInclude = false;
        options.errorHandler = nullptr;
        try
        {
            beresta::XmlParser(options).parse(text);
        }
        catch (const beresta::DomException& problem)
        {
            std::fprintf(stderr, "повторный разбор не удался: %s\n---\n%s\n---\n",
                problem.what(), text.c_str());
            std::abort();
        }
    }

    // DOM-разбор; ошибки валидности собираются, а не бросаются (так обходится весь документ)
    // DOM parsing; validity errors are collected, not thrown (so the whole document is traversed)
    void runDom(std::string_view text, beresta::ParserOptions options)
    {
        beresta::CollectingErrorHandler collector;

        options.errorHandler = &collector;
        try
        {
            beresta::Document document = beresta::XmlParser(options).parse(text);

            checkRoundTrip(document, options);
        }
        catch (const beresta::DomException&)
        {
            // Отказ разобрать некорректный ввод — штатное поведение
            // Refusing to parse invalid input is the regular behavior
        }
    }

    // SAX-разбор теми же настройками (XInclude в SAX-режиме не применяется)
    // SAX parsing with the same settings (XInclude is not applied in SAX mode)
    void runSax(std::string_view text, beresta::ParserOptions options)
    {
        beresta::ContentHandler handler;
        beresta::CollectingErrorHandler collector;

        options.errorHandler = &collector;
        options.processXInclude = false;
        try
        {
            beresta::SaxParser(options).parse(text, handler);
        }
        catch (const beresta::DomException&)
        {
        }
    }
}

// Точка входа libFuzzer: один вход — один DOM-разбор и один SAX-разбор
// The libFuzzer entry point: one input — one DOM parse and one SAX parse
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if (size == 0)
        return 0;
    std::string_view input(reinterpret_cast<const char*>(data + 1), size - 1);
    std::vector<std::string_view> parts = fuzz::splitParts(input, 2);
    std::string_view external;

    if (parts.size() > 1)
        external = parts[1];
    fuzz::MemoryResolver resolver(external);
    beresta::ParserOptions options = fuzz::optionsFromFlags(data[0]);

    options.resolver = &resolver;
    runDom(parts[0], options);
    runSax(parts[0], options);
    return 0;
}
