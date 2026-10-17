#ifndef RESOURCE_RESOLVER_HPP
#define RESOURCE_RESOLVER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace beresta
{
    // Вид запрашиваемого внешнего ресурса
    enum class ResourceKind
    {
        EXTERNAL_DTD,       // Внешнее подмножество DTD (DOCTYPE ... SYSTEM "x.dtd")
        EXTERNAL_ENTITY,    // Внешняя сущность (<!ENTITY x SYSTEM "x.xml">)
        XINCLUDE,           // Документ или текст из <xi:include href="...">
        SCHEMA              // Схема из xs:include / xs:import
    };

    // Запрос ресурса: что нужно и относительно какого документа
    struct ResourceRequest
    {
        ResourceKind kind;
        std::string_view publicId;
        std::string_view systemId;
        std::string_view baseUri;
    };

    // Загруженный ресурс: байты и его собственный URI (база для его вложенных ссылок)
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
    */
    class FileResourceResolver : public ResourceResolver
    {
    public:
        explicit FileResourceResolver(std::filesystem::path rootDirectory);

        std::optional<Resource> resolve(const ResourceRequest& request) override;

    private:
        // Абсолютный путь ресурса или nullopt, если он вне корня или сетевой
        std::optional<std::filesystem::path> locate(const ResourceRequest& request) const;

        std::filesystem::path _root;
    };
}

#endif
