#include "xsts/catalog.hpp"

#include "beresta.hpp"
#include "utils/xml_tokens.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>

namespace xsts
{
    namespace detail
    {
        // Элементы-дети в пространстве XSTS: текст и аннотации чужих пространств пропускаются
        // The child elements in the XSTS namespace: text and annotations of other namespaces are skipped
        std::vector<const beresta::Element*> childElements(const beresta::Element& parent,
            std::string_view localName)
        {
            std::vector<const beresta::Element*> found;

            for (std::size_t index = 0; index < parent.getChildCount(); ++index)
            {
                const beresta::Node* child = parent.getChild(index);

                if (child->getType() != beresta::NodeType::ELEMENT)
                    continue;
                const auto& element = static_cast<const beresta::Element&>(*child);
                if (element.getLocalName() == localName && element.getNamespaceURI() == XSTS_NAMESPACE)
                    found.push_back(&element);
            }
            return found;
        }

        /*
        ** Описание группы — текст первого annotation/documentation с
        ** пробелами, схлопнутыми в один, не длиннее 160 байт (обрезка
        ** не разрывает многобайтовый символ UTF-8).
        **
        ** The group description is the text of the first annotation/documentation with
        ** the spaces collapsed into one, at most 160 bytes long (the cut
        ** does not split a multibyte UTF-8 character).
        */
        std::string groupDescription(const beresta::Element& group)
        {
            const std::size_t LIMIT = 160;
            std::string source;
            std::string text;

            for (const beresta::Element* annotation : childElements(group, "annotation"))
            {
                for (const beresta::Element* documentation : childElements(*annotation, "documentation"))
                {
                    source = documentation->getTextContent();
                    break;
                }
                break;
            }
            for (std::string_view word : beresta::utils::splitTokens(source))
                text += std::string(word) + " ";
            std::size_t cut = text.size();
            if (cut > LIMIT)
                cut = LIMIT;
            // Байт продолжения UTF-8 (10xxxxxx) — середина символа: режем раньше
            // A UTF-8 continuation byte (10xxxxxx) is the middle of a character: cut earlier
            while (cut > 0 && cut < text.size() && (static_cast<unsigned char>(text[cut]) & 0xC0) == 0x80)
                --cut;
            text.resize(cut);
            while (!text.empty() && text.back() == ' ')
                text.pop_back();
            return text;
        }

        // Атрибуты метаданных (name, status, validity) — без префикса
        // The metadata attributes (name, status, validity) have no prefix
        std::string attributeOrEmpty(const beresta::Element& element, std::string_view name)
        {
            std::optional<std::string> value = element.getAttribute(name);

            if (!value.has_value())
                return std::string();
            return *value;
        }

        /*
        ** Метки возможностей — простой поиск подстрок в тексте документов.
        ** Это не разбор XML, а грубый фильтр для таблицы «по возможностям»:
        ** ложное срабатывание (слово в комментарии) лишь добавит случай
        ** в соседнюю строку отчёта и на вердикт не влияет.
        **
        ** The feature labels are a plain substring search in the document text.
        ** This is not XML parsing but a rough filter for the "by features" table:
        ** a false hit (a word in a comment) only adds the case
        ** to a neighbouring report row and does not affect the verdict.
        */
        std::vector<std::string> detectFeatures(const std::vector<std::filesystem::path>& documents)
        {
            static const char* const MARKERS[][2] = {
                {"substitutionGroup", "substitutionGroup"}, {"xsi:type", "xsi:type"},
                {" block=", "block/final"}, {" final=", "block/final"}, {"blockDefault=", "block/final"},
                {"finalDefault=", "block/final"}, {"abstract=", "abstract"}, {":redefine", "redefine"},
                {"<redefine", "redefine"}
            };
            std::vector<std::string> features;

            for (const std::filesystem::path& document : documents)
            {
                std::string text = beresta::io::readFile(document);

                for (const auto& marker : MARKERS)
                {
                    bool known = std::find(features.begin(), features.end(), marker[1]) != features.end();

                    if (!known && text.find(marker[0]) != std::string::npos)
                        features.push_back(marker[1]);
                }
            }
            return features;
        }

        // Каталог данных из двух частей пути: "msData/element", "sunData/ElemDecl"
        // The data directory of two path parts: "msData/element", "sunData/ElemDecl"
        std::string dataDirectory(const std::filesystem::path& file, const std::filesystem::path& root)
        {
            std::filesystem::path relative = file.lexically_relative(root);
            std::string directory;
            int parts = 0;

            for (auto part = relative.begin(); part != relative.end() && parts < 2; ++part, ++parts)
            {
                if (std::next(part) == relative.end())
                    break;
                if (!directory.empty())
                    directory += "/";
                directory += part->generic_u8string();
            }
            return directory;
        }
    }
}
