#include "conformance/catalog.hpp"
#include "conformance/conformance_error.hpp"

#include "beresta.hpp"

#include <string>

namespace conformance
{
    /*
    ** Разбор главного файла каталога. Нужны внешний DTD (testcases.dtd —
    ** без него не раскрыть сущности &sun-valid; и т.д.) и резолвер,
    ** ограниченный каталогом набора. Проверка пространств имён выключена:
    ** каталог использует только xml:base, а правила NS тут не важны.
    **
    ** Parses the main catalog file. The external DTD is needed (testcases.dtd —
    ** without it the &sun-valid; and other entities cannot be expanded) and a resolver
    ** limited to the suite directory. The namespace check is disabled:
    ** the catalog uses only xml:base, and the NS rules do not matter here.
    */
    Catalog loadCatalog(const std::filesystem::path& catalogFile)
    {
        beresta::FileResourceResolver resolver(catalogFile.parent_path());
        beresta::ParserOptions options;
        Catalog catalog;

        options.loadExternalDtd = true;
        options.namespaceAware = false;
        options.resolver = &resolver;
        options.maxEntityExpansions = 1000000;
        options.maxExpandedSize = 256u * 1024u * 1024u;
        try
        {
            beresta::Document document = beresta::XmlParser(options).parseFile(catalogFile);
            std::filesystem::path suiteRoot = catalogFile.parent_path().lexically_normal();
            std::vector<detail::EntityFile> files = detail::entityFiles(document, suiteRoot);
            detail::WalkScope scope{suiteRoot, suiteRoot, "", &files};

            detail::walkCatalog(*document.getRoot(), scope, catalog);
        }
        catch (const beresta::DomException& problem)
        {
            throw ConformanceError(std::string("не удалось прочитать каталог: ") + problem.what());
        }
        return catalog;
    }

    namespace detail
    {
        /*
        ** По XML Base у элемента внешней сущности (без своего xml:base)
        ** база — URI самой сущности. Узлы DOM не помнят, из какой
        ** сущности пришли, поэтому файлы сущностей читаются по объявлениям
        ** DOCTYPE: по тексту видно, в каком из них объявлен случай.
        **
        ** By XML Base the base of an element of an external entity (without its own xml:base)
        ** is the URI of the entity itself. DOM nodes do not remember which
        ** entity they came from, so the entity files are read by the DOCTYPE
        ** declarations: the text shows in which of them a case is declared.
        */
        std::vector<EntityFile> entityFiles(const beresta::Document& document,
            const std::filesystem::path& root)
        {
            std::vector<EntityFile> files;
            const beresta::DocumentType* doctype = document.getDocumentType();

            if (doctype == nullptr || doctype->getModel() == nullptr)
                return files;
            for (const auto& entry : doctype->getModel()->getEntities())
            {
                std::filesystem::path path = root / std::filesystem::u8path(entry.second.systemId);

                if (!entry.second.systemId.empty() && std::filesystem::is_regular_file(path))
                    files.push_back(EntityFile{path.parent_path(), beresta::io::readFile(path)});
            }
            return files;
        }

        /*
        ** Обход дерева каталога. У каждого TESTCASES может быть свой xml:base
        ** (относительный путь к папке набора) — он наследуется вглубь.
        ** Имя набора — PROFILE самого внешнего TESTCASES, где он задан:
        ** так IBM не дробится на сотню групп по продукциям грамматики.
        **
        ** The catalog tree traversal. Each TESTCASES may have its own xml:base
        ** (a relative path to the suite folder) — it is inherited downward.
        ** The suite name is the PROFILE of the outermost TESTCASES where it is set:
        ** so IBM is not split into a hundred groups by grammar productions.
        */
        void walkCatalog(const beresta::Element& element, const WalkScope& scope,
            Catalog& catalog)
        {
            WalkScope inner = scope;
            std::string base = attributeOrEmpty(element, "xml:base");
            std::string profile = attributeOrEmpty(element, "PROFILE");

            if (!base.empty())
                inner.base = scope.base / std::filesystem::u8path(base);
            if (!profile.empty() && scope.suite.empty() && element.getName() == "TESTCASES")
                inner.suite = profile;
            if (element.getName() == "TEST")
            {
                catalog.push_back(makeTestCase(element, inner));
                return;
            }
            for (std::size_t index = 0; index < element.getChildCount(); ++index)
            {
                const beresta::Node* child = element.getChild(index);

                // Текст, комментарии и PI каталога прогонщику не нужны
                // The runner does not need the text, comments and PIs of the catalog
                if (child->getType() == beresta::NodeType::ELEMENT)
                    walkCatalog(static_cast<const beresta::Element&>(*child), inner, catalog);
            }
        }
    }
}
