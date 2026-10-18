#include "xinclude/internal/xinclude_processor.hpp"
#include "i18n/i18n.hpp"
#include "core/text.hpp"
#include "encoding/encoding_detector.hpp"
#include "exception/dom_exception.hpp"
#include "parser/xml_parser.hpp"

#include <algorithm>
#include <utility>

namespace beresta::xinclude::detail
{
    /*
    ** Содержимое для подстановки: parse="text" — один текстовый узел,
    ** parse="xml" (по умолчанию) — выбранный элемент загруженного документа
    ** с уже обработанными вложенными включениями.
    **
    ** The content to substitute: parse="text" — one text node,
    ** parse="xml" (the default) — the selected element of the loaded document
    ** with the nested inclusions already processed.
    */
    std::vector<std::unique_ptr<Node>> XIncludeProcessor::loadReplacement(const Element& include,
        const std::string& baseUri)
    {
        std::string parse = include.getAttribute("parse").value_or("xml");
        std::vector<std::unique_ptr<Node>> nodes;
        Resource resource = fetch(include, baseUri);

        if (parse == "text")
        {
            std::string encodingName = include.getAttribute("encoding").value_or("");
            encoding::DecodedDocument decoded = encoding::decodeDocument(resource.content, encodingName);
            nodes.push_back(std::make_unique<Text>(std::string(decoded.getText())));
            return nodes;
        }
        if (parse != "xml")
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::XINCLUDE_PARSE_VALUE, {parse}));
        nodes.push_back(loadXml(resource, include));
        return nodes;
    }

    // Загрузка через резолвер с проверкой циклов и глубины
    // Loading through the resolver with cycle and depth checks
    Resource XIncludeProcessor::fetch(const Element& include, const std::string& baseUri) const
    {
        std::string href = include.getAttribute("href").value_or("");

        if (href.empty())
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::XINCLUDE_NO_HREF));
        if (_options.resolver == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::XINCLUDE_RESOLVER));
        std::optional<Resource> resource = _options.resolver->resolve(
            ResourceRequest{ResourceKind::XINCLUDE, "", href, baseUri});
        if (!resource.has_value())
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::XINCLUDE_LOAD, {href}));
        if (std::find(_stack.begin(), _stack.end(), resource->uri) != _stack.end())
            throw DomException(DomErrorCode::LIMIT_EXCEEDED, i18n::format(i18n::Message::XINCLUDE_CYCLE, {href}));
        if (_stack.size() >= _options.maxEntityDepth)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED, i18n::format(i18n::Message::XINCLUDE_TOO_DEEP));
        return std::move(resource.value());
    }

    /*
    ** XML-ресурс разбирается с теми же настройками (кроме проверок схемы
    ** и самого XInclude), затем обрабатываются его собственные включения,
    ** и только после этого выбирается фрагмент по xpointer.
    **
    ** An XML resource is parsed with the same settings (except the schema checks
    ** and XInclude itself), then its own inclusions are processed,
    ** and only after that the fragment is selected by xpointer.
    */
    std::unique_ptr<Node> XIncludeProcessor::loadXml(const Resource& resource, const Element& include)
    {
        ParserOptions nested = _options;
        nested.processXInclude = false;
        nested.schema = nullptr;
        nested.baseUri = resource.uri;
        Document document = XmlParser(nested).parse(resource.content);

        _stack.push_back(resource.uri);
        process(document, resource.uri);
        _stack.pop_back();
        std::optional<std::string> pointer = include.getAttribute("xpointer");
        if (pointer.has_value())
            return selectPointer(document, pointer.value());
        return document.releaseRoot();
    }
}
