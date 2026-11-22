#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

/*
** Регрессии ревью кода 2026-11-21 (задачи Т-2 и Т-5): правила xi:include,
** предел повторных включений, стек URI после ошибки и выход резолвера
** за корень по символической ссылке.
**
** Regressions of the code review of 2026-11-21 (tasks T-2 and T-5): the xi:include rules,
** the limit of repeated inclusions, the URI stack after an error and the resolver leaving
** the root through a symbolic link.
*/
namespace
{
    const char* const XI = "xmlns:xi=\"http://www.w3.org/2001/XInclude\"";

    // Разбор файла с XInclude; maxRepeats — предел повторных включений (maxEntityExpansions)
    // Parses a file with XInclude; maxRepeats is the limit of repeated inclusions (maxEntityExpansions)
    std::string includeResult(const std::filesystem::path& root, const char* file, std::size_t maxRepeats)
    {
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;
        options.processXInclude = true;
        options.maxEntityExpansions = maxRepeats;

        return beresta::XmlParser(options).parseFile(root / file).toString();
    }

    /*
    ** Два xi:fallback — ошибка, даже если ресурс есть; xi:include без href и
    ** xpointer — ошибка, а не переход к xi:fallback (XInclude 1.0, 3.1 и 3.2).
    **
    ** Two xi:fallback are an error even if the resource exists; an xi:include without href and
    ** xpointer is an error, not a fallback (XInclude 1.0, 3.1 and 3.2).
    */
    void testFallbackRules()
    {
        const support::TestDirectory temporary("review_xinclude_rules");
        const auto& root = temporary.path;
        support::writeTestFile(root / "part.xml", "<part/>");
        support::writeTestFile(root / "two.xml", std::string("<r ") + XI + "><xi:include href=\"part.xml\">"
            "<xi:fallback>1</xi:fallback><xi:fallback>2</xi:fallback></xi:include></r>");
        support::writeTestFile(root / "nohref.xml", std::string("<r ") + XI + "><xi:include>"
            "<xi:fallback>1</xi:fallback></xi:include></r>");
        support::writeTestFile(root / "one.xml", std::string("<r ") + XI + "><xi:include href=\"missing.xml\">"
            "<xi:fallback>1</xi:fallback></xi:include></r>");

        TEST_ASSERT_THROWS(includeResult(root, "two.xml", 10), beresta::DomException);
        TEST_ASSERT_THROWS(includeResult(root, "nohref.xml", 10), beresta::DomException);
        TEST_ASSERT_EQ(includeResult(root, "one.xml", 10), std::string("<r ") + XI + ">1</r>");
    }

    /*
    ** Повторы одного файла ограничены (цепочка «каждый включает следующий много
    ** раз» иначе росла бы экспоненциально); первая загрузка не считается.
    ** После ошибки внутри включённого файла и перехода к xi:fallback тот же файл
    ** включается снова, а не считается циклом.
    **
    ** Repeats of one file are limited (a chain "each one includes the next many
    ** times" would grow exponentially otherwise); the first load is not counted.
    ** After an error inside an included file and a fallback the same file
    ** is included again instead of being taken for a cycle.
    */
    void testRepeatedIncludes()
    {
        const support::TestDirectory temporary("review_xinclude_repeats");
        const auto& root = temporary.path;
        std::string five;
        for (int index = 0; index < 5; ++index)
            five += "<xi:include href=\"leaf.xml\"/>";
        support::writeTestFile(root / "leaf.xml", "<leaf/>");
        support::writeTestFile(root / "five.xml", std::string("<r ") + XI + ">" + five + "</r>");
        support::writeTestFile(root / "broken.xml", std::string("<b ") + XI + "><xi:include href=\"missing.xml\"/></b>");
        support::writeTestFile(root / "again.xml", std::string("<r ") + XI + ">"
            "<xi:include href=\"broken.xml\"><xi:fallback>f</xi:fallback></xi:include>"
            "<xi:include href=\"broken.xml\"><xi:fallback>g</xi:fallback></xi:include></r>");

        TEST_ASSERT_THROWS(includeResult(root, "five.xml", 3), beresta::DomException);
        TEST_ASSERT_EQ(includeResult(root, "five.xml", 4), std::string("<r ") + XI + "><leaf/><leaf/><leaf/><leaf/><leaf/></r>");
        TEST_ASSERT_EQ(includeResult(root, "again.xml", 10), std::string("<r ") + XI + ">fg</r>");
    }

    /*
    ** Символическая ссылка внутри корня на каталог снаружи не открывает чтение за корнем (Т-2).
    ** Где ссылки создать нельзя (Windows без прав), проверяется только обычный путь.
    **
    ** A symbolic link inside the root to a directory outside does not open reading outside the root (T-2).
    ** Where links cannot be created (Windows without rights), only the ordinary path is checked.
    */
    void testResolverSymlink()
    {
        const support::TestDirectory temporary("review_symlink");
        const auto& base = temporary.path;
        std::filesystem::path root = base / "root";
        std::filesystem::path outside = base / "outside";
        std::error_code error;

        std::filesystem::create_directories(root);
        std::filesystem::create_directories(outside);
        support::writeTestFile(outside / "secret.xml", "<secret/>");
        support::writeTestFile(root / "inside.xml", "<inside/>");
        beresta::FileResourceResolver resolver(root);
        TEST_ASSERT(resolver.resolve(beresta::ResourceRequest{beresta::ResourceKind::XINCLUDE, "", "inside.xml", ""})
            .has_value());
        std::filesystem::create_directory_symlink(outside, root / "external", error);
        if (error)
            return;
        TEST_ASSERT(!resolver.resolve(beresta::ResourceRequest{beresta::ResourceKind::XINCLUDE, "",
            "external/secret.xml", ""}).has_value());
    }
}

namespace suites
{
    // Регистрация регрессий ревью: XInclude и резолвер
    // Registers the review regressions: XInclude and the resolver
    void registerReviewXIncludeTests(test::TestRunner& runner)
    {
        runner.add("Ревью Т-5: правила xi:include и xi:fallback", testFallbackRules);
        runner.add("Ревью Т-5: повторные включения и стек URI", testRepeatedIncludes);
        runner.add("Ревью Т-2: резолвер и символические ссылки", testResolverSymlink);
    }
}
