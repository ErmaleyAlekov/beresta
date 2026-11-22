#include "bench/engine.hpp"

#include <iostream>

namespace
{
    const std::string VALID = "<root>7</root>";
    const std::string INVALID = "<root>word</root>";
    const std::string INCOMPLETE = "<root>";
    const std::string SCHEMA =
        "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
        "<xs:element name='root' type='xs:int'/></xs:schema>";

    // Проверяем завершение операции ошибкой разбора / Verify that the operation reports a parse error
    template<class Operation>
    void expectRejection(Operation operation, const std::string& context)
    {
        try
        {
            operation();
        }
        catch (const bench::BenchError&)
        {
            return;
        }
        throw bench::BenchError(context + ": XML принят / XML accepted");
    }

    // После неудачного разбора предыдущий документ можно обойти и сериализовать.
    // A parse failure leaves the previous document available for walking and serialization.
    void checkLifetime(bench::XmlEngine& engine)
    {
        engine.parse(VALID);
        const auto checksum = engine.traverse();
        const auto bytes = engine.serialize();
        expectRejection([&engine] { engine.parse(INCOMPLETE); }, engine.name());
        if (engine.traverse() != checksum || engine.serialize() != bytes)
            throw bench::BenchError(engine.name() + ": дерево изменилось / tree changed");
        engine.release();
        engine.release();
        expectRejection([&engine] { engine.parse(INCOMPLETE); }, engine.name());
        if (engine.traverse() != 0 || engine.serialize() != 0)
            throw bench::BenchError(engine.name() + ": ожидалось пустое дерево / expected empty tree");
        engine.parse(VALID);
        if (engine.traverse() != checksum)
            throw bench::BenchError(engine.name() + ": обход после повтора / traversal after retry");
    }

    // Проверяем валидность, синтаксический отказ и последующее переиспользование.
    // Check validation, syntax rejection, and successful reuse afterward.
    void checkValidation(bench::XmlEngine& engine, bool streaming)
    {
        const auto invalid = engine.parseValidated(INVALID, SCHEMA);
        if (!invalid || *invalid == 0)
            throw bench::BenchError(engine.name() + ": пропущена ошибка XSD / missed XSD error");
        expectRejection(
            [&engine] { engine.parseValidated(INCOMPLETE, SCHEMA); }, engine.name());
        if (engine.parseValidated(VALID, SCHEMA) != 0)
            throw bench::BenchError(engine.name() + ": повторная проверка / validation retry");
        if (!streaming)
            return;
        const auto streamInvalid = engine.streamValidated(INVALID, SCHEMA);
        if (!streamInvalid || *streamInvalid == 0 || engine.streamValidated(VALID, SCHEMA) != 0)
            throw bench::BenchError(engine.name() + ": потоковая проверка / stream validation");
    }

    // Проверяем различия адаптеров на минимальных локальных документах.
    // Check adapter-specific behavior with minimal local documents.
    void checkEngine(bench::XmlEngine& engine)
    {
        const std::string name = engine.name();
        engine.parse(VALID);
        if (engine.traverse() != 2)
            throw bench::BenchError(name + ": базовый обход / basic traversal");
        if (name.find("pugixml") == 0)
        {
            engine.parse("<a>\n  <b/>\n</a>");
            if (engine.traverse() != 4)
                throw bench::BenchError(name + ": пробельные узлы / whitespace nodes");
        }
        if (name.find("RapidXML") == 0)
        {
            engine.parse("<root/>");
            const auto bare = engine.serialize();
            engine.parse("<?xml version=\"1.0\"?><root/>");
            const auto declared = engine.serialize();
            if (!bare || !declared || *declared <= *bare)
                throw bench::BenchError(name + ": декларация / XML declaration");
        }
        if (name.find("libxml2") == 0 || name.find("Xerces") == 0)
        {
            checkLifetime(engine);
            checkValidation(engine, name.find("libxml2") == 0);
        }
    }
}

// Запускаем проверки на адаптерах, включённых в сборку / Run checks against the compiled-in adapters
int main()
{
    try
    {
        auto engines = bench::createEngines();
        for (auto& engine : engines)
            checkEngine(*engine);
        return 0;
    }
    catch (const std::exception& problem)
    {
        std::cerr << problem.what() << '\n';
        return 1;
    }
}
