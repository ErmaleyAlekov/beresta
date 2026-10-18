#ifndef FUZZ_INPUT_HPP
#define FUZZ_INPUT_HPP

#include "parser/parser_options.hpp"
#include "parser/resource_resolver.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fuzz
{
    /*
    ** Разделитель частей входа фаззера. Байт 0xF5 никогда не встречается
    ** в корректном UTF-8, поэтому пара 0xF5 0xF5 не мешает мутациям
    ** самого XML; в корпусе и словаре он присутствует, так что libFuzzer
    ** быстро учится порождать многочастные входы.
    **
    ** The separator of the fuzzer input parts. The byte 0xF5 never occurs
    ** in valid UTF-8, so the pair 0xF5 0xF5 does not interfere with mutations
    ** of the XML itself; it is present in the corpus and the dictionary, so libFuzzer
    ** quickly learns to produce multi-part inputs.
    */
    inline constexpr std::string_view PART_SEPARATOR = "\xF5\xF5";

    // Делит вход на не более чем maxParts частей по PART_SEPARATOR
    // Splits the input into at most maxParts parts by PART_SEPARATOR
    std::vector<std::string_view> splitParts(std::string_view input, std::size_t maxParts);

    /*
    ** Байт флагов -> настройки парсера. Каждый бит включает одну
    ** возможность, чтобы фаззер перебирал их сочетания:
    **   0 — namespaceAware, 1 — validateDtd, 2 — loadExternalDtd,
    **   3 — preserveWhitespaceText, 4 — coalesceCData, 5 — trackLocations,
    **   6 — applyDtdDefaults, 7 — processXInclude.
    ** Лимиты уменьшены: фаззер ищет падения, а не медленные входы.
    **
    ** The flag byte -> the parser settings. Each bit enables one
    ** feature so that the fuzzer tries their combinations:
    ** 0 — namespaceAware, 1 — validateDtd, 2 — loadExternalDtd,
    ** 3 — preserveWhitespaceText, 4 — coalesceCData, 5 — trackLocations,
    ** 6 — applyDtdDefaults, 7 — processXInclude.
    ** The limits are reduced: the fuzzer looks for crashes, not slow inputs.
    */
    beresta::ParserOptions optionsFromFlags(std::uint8_t flags);

    /*
    ** MemoryResolver — «внешний мир» фаззера: на ЛЮБОЙ запрос (DTD,
    ** сущность, XInclude, схема) отдаёт один и тот же текст из входа.
    ** Так фаззер управляет и документом, и внешними ресурсами, а
    ** настоящая файловая система не трогается.
    **
    ** MemoryResolver — the fuzzer's "outside world": for ANY request (a DTD,
    ** an entity, XInclude, a schema) it returns the same text from the input.
    ** So the fuzzer controls both the document and the external resources, and
    ** the real file system is not touched.
    */
    class MemoryResolver : public beresta::ResourceResolver
    {
    public:
        explicit MemoryResolver(std::string_view content);

        std::optional<beresta::Resource> resolve(const beresta::ResourceRequest& request) override;

    private:
        std::string _content;
    };
}

#endif
