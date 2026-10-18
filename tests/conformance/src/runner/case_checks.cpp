#include "conformance/case_runner.hpp"
#include "conformance/canonical_writer.hpp"
#include "conformance/conformance_error.hpp"

#include "beresta.hpp"

#include <algorithm>
#include <filesystem>
#include <string>

namespace conformance
{
    namespace
    {
        // Длина фрагмента вокруг первого расхождения в отчёте
        // The length of the fragment around the first mismatch in the report
        constexpr std::size_t EXCERPT_LENGTH = 40;

        // Фрагмент строки начиная с позиции from (видимые \n вместо переводов строк)
        // A fragment of the string starting from position from (visible \n instead of line breaks)
        std::string excerpt(const std::string& text, std::size_t from)
        {
            std::string result;
            std::size_t end = std::min(text.size(), from + EXCERPT_LENGTH);

            for (std::size_t index = from; index < end; ++index)
            {
                if (text[index] == '\n')
                    result += "\\n";
                else
                    result += text[index];
            }
            return result;
        }
    }

    /*
    ** Область применимости: библиотека реализует XML 1.0 5-й редакции и
    ** Namespaces 1.0. Случаи XML 1.1 / NS 1.1 и случаи, помеченные
    ** только для редакций 1–4 (правила имён там другие), пропускаются —
    ** так же поступают прогоны xmlconf у libxml2 и Xerces.
    **
    ** The scope: the library implements XML 1.0 5th edition and
    ** Namespaces 1.0. The XML 1.1 / NS 1.1 cases and the cases marked
    ** only for editions 1–4 (the name rules are different there) are skipped —
    ** the xmlconf runs of libxml2 and Xerces do the same.
    */
    std::string skipReason(const TestCase& testCase)
    {
        const std::string& recommendation = testCase.recommendation;

        if (recommendation == "XML1.1" || recommendation == "NS1.1" || testCase.version == "1.1")
            return "XML 1.1 / Namespaces 1.1 не поддерживаются";
        if (!testCase.edition.empty() && testCase.edition.find('5') == std::string::npos)
            return "случай только для редакций 1-4 XML 1.0 (библиотека следует 5-й)";
        // Каталог 2013 года ссылается на пару файлов, которых нет в архиве
        // The 2013 catalog refers to a couple of files that are missing from the archive
        if (!std::filesystem::exists(testCase.file))
            return "файл случая отсутствует в архиве набора";
        return std::string();
    }

    /*
    ** Сравнение с OUTPUT. Блок DOCTYPE второй канонической формы срезается
    ** у ожидаемого текста, так что сравнивается тело документа; второй
    ** вариант — без PI из DTD (их выносят перед DOCTYPE выводы IBM). При
    ** расхождении в отчёт идут оба фрагмента от первой отличающейся позиции.
    **
    ** Comparison with OUTPUT. The DOCTYPE block of the second canonical form is cut off
    ** from the expected text, so the document body is compared; the second
    ** variant is without the PIs from the DTD (the IBM outputs move them before the DOCTYPE). On a
    ** mismatch both fragments from the first differing position go into the report.
    */
    std::string compareOutput(const TestCase& testCase, const beresta::Document& document)
    {
        std::string raw;

        try
        {
            raw = beresta::io::readFile(testCase.output);
        }
        catch (const beresta::DomException& problem)
        {
            throw ConformanceError("не читается файл OUTPUT случая " + testCase.id + ": " + problem.what());
        }
        std::string expected = detail::stripCanonicalDoctype(raw, false);
        std::string actual = writeCanonical(document);
        if (actual == expected || actual == detail::stripCanonicalDoctype(raw, true))
            return std::string();
        std::size_t position = 0;
        while (position < actual.size() && position < expected.size()
            && actual[position] == expected[position])
            ++position;
        return "канонический вывод отличается с байта " + std::to_string(position)
            + ": ожидалось \"" + excerpt(expected, position) + "\", получено \""
            + excerpt(actual, position) + "\"";
    }
}
