#include "capi/internal/capi_support.hpp"
#include "core/cdata_section.hpp"
#include "core/processing_instruction.hpp"

// Секция CDATA: "]]>" внутри текста отвергается ядром (INVALID_CONTENT)
extern "C" BERESTA_API beresta_node beresta_node_append_cdata(beresta_node parent, const char* text)
{
    BERESTA_CAPI_TRY
        std::string value(beresta::capi::requireText(text, "text"));

        return beresta::capi::appendNode(parent, std::make_unique<beresta::CDataSection>(std::move(value)));
    BERESTA_CAPI_CATCH(nullptr)
}

// Инструкция обработки; data == NULL — инструкция без данных
extern "C" BERESTA_API beresta_node beresta_node_append_pi(beresta_node parent, const char* target,
    const char* data)
{
    BERESTA_CAPI_TRY
        std::string name(beresta::capi::requireText(target, "target"));
        std::string value;

        if (data != nullptr)
            value = data;
        return beresta::capi::appendNode(parent,
            std::make_unique<beresta::ProcessingInstruction>(std::move(name), std::move(value)));
    BERESTA_CAPI_CATCH(nullptr)
}
