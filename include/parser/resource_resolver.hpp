#ifndef RESOURCE_RESOLVER_HPP
#define RESOURCE_RESOLVER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace beresta
{
    // Вид запрашиваемого внешнего ресурса
    // The kind of the requested external resource
    enum class ResourceKind
    {
        EXTERNAL_DTD,       // Внешнее подмножество DTD (DOCTYPE ... SYSTEM "x.dtd")
                            // The external DTD subset (DOCTYPE ... SYSTEM "x.dtd")
        EXTERNAL_ENTITY,    // Внешняя сущность (<!ENTITY x SYSTEM "x.xml">)
                            // An external entity (<!ENTITY x SYSTEM "x.xml">)
        XINCLUDE,           // Документ или текст из <xi:include href="...">
                            // A document or text from <xi:include href="...">
        SCHEMA              // Схема из xs:include / xs:import / A schema from xs:include / xs:import
    };

    // Запрос ресурса: что нужно и относительно какого документа
    // A resource request: what is needed and relative to which document
    struct ResourceRequest
    {
        ResourceKind kind;
        std::string_view publicId;
        std::string_view systemId;
        std::string_view baseUri;
    };

    // Загруженный ресурс: байты и его собственный URI (база для его вложенных ссылок)
    // A loaded resource: the bytes and its own URI (the base for its nested references)
    struct Resource
    {
        std::string content;
        std::string uri;
    };

    /*
    ** ResourceResolver — стратегия загрузки внешних ресурсов (паттерн Strategy,
    ** аналог EntityResolver в Xerces). По умолчанию в парсере резолвера нет,
    ** и НИЧЕГО внешнего не загружается: это закрывает уязвимости XXE и SSRF.
    ** Вернуть std::nullopt — значит «ресурс недоступен».
    **
    ** ResourceResolver — a strategy for loading external resources (the Strategy pattern,
    ** the analog of EntityResolver in Xerces). By default the parser has no resolver,
    ** and NOTHING external is loaded: this closes the XXE and SSRF vulnerabilities.
    ** Returning std::nullopt means "the resource is unavailable".
    */
    class ResourceResolver
    {
    public:
        virtual ~ResourceResolver();

        virtual std::optional<Resource> resolve(const ResourceRequest& request) = 0;
    };

    /*
    ** FileResourceResolver — загрузка ресурсов только с локального диска
    ** и только внутри указанного корневого каталога. Сетевые URI (http:,
    ** https:, ftp: и т.п.) и пути, выходящие за корень через "..", отвергаются.
    **
    ** FileResourceResolver — loads resources only from the local disk
    ** and only inside the given root directory. Network URIs (http:,
    ** https:, ftp: and so on) and paths leaving the root through ".." are rejected.
    */
    class FileResourceResolver : public ResourceResolver
    {
    public:
        explicit FileResourceResolver(std::filesystem::path rootDirectory);

        std::optional<Resource> resolve(const ResourceRequest& request) override;

    private:
        // Абсолютный путь ресурса или nullopt, если он вне корня или сетевой
        // The absolute path of the resource or nullopt if it is outside the root or a network one
        std::optional<std::filesystem::path> locate(const ResourceRequest& request) const;

        std::filesystem::path _root;
    };
}

#endif
