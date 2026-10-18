#include "bench/workload.hpp"

#include <string>

namespace bench
{
    // Фрагменты русского текста по кругу / Fragments of Russian text in a circle
    static const char* const WORDS[] = {
        "На бересте писали грамоты, письма и счета",
        "а береста хранила текст столетиями",
        "документы разбирали, переписывали &amp; сверяли с оригиналом",
        "каждая строка требовала внимания"
    };

    /*
    ** Абзац со смешанным содержимым: текст, вложенные <b> и <i>, ссылка на
    ** сущность. Это типичный «документный» XML (статьи, книги, справка),
    ** где основная работа парсера — текст, а не теги.
    **
    ** A paragraph with mixed content: text, nested <b> and <i>, an entity
    ** reference. This is typical "document" XML (articles, books, help files),
    ** where the parser's main work is text, not tags.
    */
    static void appendParagraph(std::string& xml, std::size_t index)
    {
        xml += "<p n=\"" + std::to_string(index) + "\">";
        for (std::size_t part = 0; part < 6; ++part)
        {
            xml += WORDS[(index + part) % 4];
            xml += ", ";
        }
        xml += "<b>выделено ";
        xml += std::to_string(index);
        xml += "</b> и <i>курсив</i> &lt;конец&gt;.</p>";
    }

    // Статья из paragraphs абзацев, по 20 абзацев в разделе / An article of paragraphs paragraphs, 20 per section
    Workload makeArticle(std::size_t paragraphs)
    {
        Workload workload;

        workload.name = "article";
        workload.xml.reserve(paragraphs * 400);
        workload.xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><article><title>Берёста</title>";
        for (std::size_t index = 0; index < paragraphs; ++index)
        {
            if (index % 20 == 0)
                workload.xml += "<section><title>Раздел " + std::to_string(index / 20) + "</title>";
            appendParagraph(workload.xml, index);
            if (index % 20 == 19 || index + 1 == paragraphs)
                workload.xml += "</section>";
        }
        workload.xml += "</article>";
        workload.query = "//section/p[b]";
        return workload;
    }
}
