#include "parser/resource_resolver.hpp"
#include "io/file_io.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Виртуальный деструктор интерфейса
    ResourceResolver::~ResourceResolver()
    {
    }

    // Корень нормализуется один раз, чтобы сравнение префиксов было надёжным
    FileResourceResolver::FileResourceResolver(std::filesystem::path rootDirectory)
        : _root(std::filesystem::absolute(std::move(rootDirectory)).lexically_normal())
    {
    }

    // Схема URI ("http:", "file:") — буквы и ':' до первого '/'; одиночная буква — диск Windows
    static bool hasUriScheme(std::string_view id) noexcept
    {
        std::size_t colon = id.find(':');

        if (colon == std::string_view::npos || colon < 2)
            return false;
        return id.find('/') == std::string_view::npos || colon < id.find('/');
    }

    /*
    ** Относительный systemId разрешается от каталога базового документа.
    ** Итоговый путь обязан лежать внутри корня — иначе запрос отвергается.
    */
    std::optional<std::filesystem::path> FileResourceResolver::locate(
        const ResourceRequest& request) const
    {
        if (request.systemId.empty() || hasUriScheme(request.systemId))
            return std::nullopt;
        std::filesystem::path base = _root;
        if (!request.baseUri.empty())
            base = std::filesystem::absolute(std::filesystem::u8path(request.baseUri)).parent_path();
        std::filesystem::path target =
            (base / std::filesystem::u8path(request.systemId)).lexically_normal();
        std::filesystem::path relative = target.lexically_relative(_root);
        if (relative.empty() || *relative.begin() == std::filesystem::path(".."))
            return std::nullopt;
        return target;
    }

    // Чтение файла; ошибка ввода-вывода означает «ресурс недоступен»
    std::optional<Resource> FileResourceResolver::resolve(const ResourceRequest& request)
    {
        std::optional<std::filesystem::path> path = locate(request);

        if (!path.has_value())
            return std::nullopt;
        try
        {
            return Resource{io::readFile(path.value()), path->u8string()};
        }
        catch (const DomException&)
        {
            return std::nullopt;
        }
    }
}
