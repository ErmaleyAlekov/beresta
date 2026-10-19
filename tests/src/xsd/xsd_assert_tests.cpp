#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Пример пользователя (schema1.xml): банковская выписка XSD 1.1 с
    ** vc:minVersion="1.1" и двумя xs:assert. Первое: без транзакций
    ** начальный и конечный балансы равны. Второе: начальный баланс плюс
    ** приход (cr) минус расход (dr) равен конечному — суммы xs:decimal.
    **
    ** The user's example (schema1.xml): an XSD 1.1 bank statement with
    ** vc:minVersion="1.1" and two xs:assert. The first one: without transactions
    ** the opening and closing balances are equal. The second one: the opening balance plus
    ** the credit (cr) minus the debit (dr) equals the closing one — xs:decimal sums.
    */
    const char* const STATEMENT_XSD =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n"
        "           xmlns:vc=\"http://www.w3.org/2007/XMLSchema-versioning\"\n"
        "           elementFormDefault=\"qualified\"\n"
        "           vc:minVersion=\"1.1\">\n"
        "\n"
        "  <xs:element name=\"Statement\">\n"
        "    <xs:complexType>\n"
        "      <xs:sequence>\n"
        "        <xs:element name=\"AccountInfo\">\n"
        "          <xs:complexType>\n"
        "            <xs:sequence>\n"
        "              <xs:element name=\"saldoAwal\" type=\"xs:decimal\"/>\n"
        "              <xs:element name=\"saldoAkhir\" type=\"xs:decimal\"/>\n"
        "            </xs:sequence>\n"
        "          </xs:complexType>\n"
        "        </xs:element>\n"
        "        <xs:element name=\"Transactions\" minOccurs=\"0\">\n"
        "          <xs:complexType>\n"
        "            <xs:sequence>\n"
        "              <xs:element name=\"Tx\" maxOccurs=\"unbounded\">\n"
        "                <xs:complexType>\n"
        "                  <xs:sequence>\n"
        "                    <xs:element name=\"type\" type=\"xs:string\"/> <!-- 'cr' или 'dr' -->\n"
        "                    <xs:element name=\"amount\" type=\"xs:decimal\"/>\n"
        "                  </xs:sequence>\n"
        "                </xs:complexType>\n"
        "              </xs:element>\n"
        "            </xs:sequence>\n"
        "          </xs:complexType>\n"
        "        </xs:element>\n"
        "      </xs:sequence>\n"
        "      \n"
        "      <!-- Утверждение 1: Если транзакций нет, балансы обязаны быть равны -->\n"
        "      <xs:assert test=\"if (not(Transactions/Tx)) then AccountInfo/saldoAwal = AccountInfo/saldoAkhir"
        " else true()\"/>\n"
        "      \n"
        "      <!-- Утверждение 2: Математический баланс сходимости транзакций -->\n"
        "      <xs:assert test=\"if (Transactions/Tx) then (AccountInfo/saldoAwal + sum(Transactions/Tx[type='cr']/amount)"
        " - sum(Transactions/Tx[type='dr']/amount) = AccountInfo/saldoAkhir) else true()\"/>\n"
        "      \n"
        "    </xs:complexType>\n"
        "  </xs:element>\n"
        "</xs:schema>\n";

    // Документ пользователя (schema2.xml): 1000.00 + 500.00 − 200.00 = 1300.00
    // The user's document (schema2.xml): 1000.00 + 500.00 − 200.00 = 1300.00
    const char* const STATEMENT_XML =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Statement>\n"
        "  <AccountInfo>\n"
        "    <saldoAwal>1000.00</saldoAwal>\n"
        "    <saldoAkhir>1300.00</saldoAkhir>\n"
        "  </AccountInfo>\n"
        "  <Transactions>\n"
        "    <Tx>\n"
        "      <type>cr</type>\n"
        "      <amount>500.00</amount>\n"
        "    </Tx>\n"
        "    <Tx>\n"
        "      <type>dr</type>\n"
        "      <amount>200.00</amount>\n"
        "    </Tx>\n"
        "  </Transactions>\n"
        "</Statement>\n";

    /*
    ** Выписка из балансов и транзакций "вид:сумма;вид:сумма" (пустая строка —
    ** без элемента Transactions). Корень — на второй строке, с первого столбца.
    **
    ** A statement from balances and transactions "kind:amount;kind:amount" (an empty string —
    ** without the Transactions element). The root is on the second line, from the first column.
    */
    std::string statement(const std::string& opening, const std::string& closing, const std::string& transactions)
    {
        std::string xml = "<?xml version=\"1.0\"?>\n<Statement><AccountInfo><saldoAwal>" + opening
            + "</saldoAwal><saldoAkhir>" + closing + "</saldoAkhir></AccountInfo>";
        std::size_t start = 0;

        if (!transactions.empty())
            xml += "<Transactions>";
        while (start < transactions.size())
        {
            std::size_t end = transactions.find(';', start);
            if (end == std::string::npos)
                end = transactions.size();
            std::string entry = transactions.substr(start, end - start);
            std::size_t colon = entry.find(':');
            xml += "<Tx><type>" + entry.substr(0, colon) + "</type><amount>" + entry.substr(colon + 1)
                + "</amount></Tx>";
            start = end + 1;
        }
        if (!transactions.empty())
            xml += "</Transactions>";
        return xml + "</Statement>";
    }

    /*
    ** Примеры пользователя: схема с vc:minVersion="1.1" загружается, а
    ** документ проходит во всех трёх режимах (SAX, разбор в DOM, готовое
    ** дерево). Те же балансы без транзакций с разной записью одного числа
    ** (1000.00 и 1000) равны: сравниваются значения xs:decimal, а не строки.
    **
    ** The user's examples: the schema with vc:minVersion="1.1" loads, and
    ** the document passes in all three modes (SAX, parsing into a DOM, a ready
    ** tree). The same balances without transactions with different notations of one number
    ** (1000.00 and 1000) are equal: xs:decimal values are compared, not strings.
    */
    void testUserExample()
    {
        support::SchemaRuns runs = support::runSchemaModes(STATEMENT_XSD, STATEMENT_XML);

        TEST_ASSERT_EQ(support::describeDiagnostics(runs.sax), "");
        TEST_ASSERT_EQ(support::describeDiagnostics(runs.parsed), "");
        TEST_ASSERT_EQ(support::describeDiagnostics(runs.tree), "");
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD, statement("1000.00", "1000", "")).empty());
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD, statement("-5", "-5.0", "")).empty());
    }

    /*
    ** Нарушения: неверный конечный баланс — ложно второе утверждение;
    ** нет транзакций, а балансы разные — первое. Ошибка относится к
    ** /Statement (строка 2, столбец 1) и одинакова во всех трёх режимах.
    **
    ** Violations: a wrong closing balance — the second assertion is false;
    ** there are no transactions but the balances differ — the first one. The error refers to
    ** /Statement (line 2, column 1) and is the same in all three modes.
    */
    void testBalanceViolations()
    {
        std::vector<beresta::Diagnostic> wrong = support::agreedSchemaErrors(STATEMENT_XSD,
            statement("1000.00", "1400.00", "cr:500.00;dr:200.00"));
        std::vector<beresta::Diagnostic> empty = support::agreedSchemaErrors(STATEMENT_XSD,
            statement("1000.00", "900.00", ""));

        TEST_ASSERT(wrong.size() == 1);
        TEST_ASSERT(wrong[0].message.rfind("/Statement: не выполнено утверждение if (Transactions/Tx) then", 0) == 0);
        TEST_ASSERT_EQ(std::to_string(wrong[0].position.line) + ":" + std::to_string(wrong[0].position.column), "2:1");
        TEST_ASSERT(empty.size() == 1);
        TEST_ASSERT(empty[0].message.rfind("/Statement: не выполнено утверждение if (not(Transactions/Tx))", 0) == 0);
        TEST_ASSERT_EQ(std::to_string(empty[0].position.line) + ":" + std::to_string(empty[0].position.column), "2:1");
    }

    /*
    ** Точная арифметика: 0.10 + 0.20 = 0.30 (в double это 0.30000000000000004),
    ** десять приходов по 0.10 дают ровно 1.00, а суммы за пределами точности
    ** double (90071992547409.93 + 0.01) сходятся до копейки. Расхождение в
    ** одну копейку — нарушение.
    **
    ** Exact arithmetic: 0.10 + 0.20 = 0.30 (in double it is 0.30000000000000004),
    ** ten credits of 0.10 give exactly 1.00, and sums beyond the precision of
    ** double (90071992547409.93 + 0.01) match to the cent. A mismatch of
    ** one cent is a violation.
    */
    void testExactCents()
    {
        std::string tenCents;

        for (int index = 0; index < 10; ++index)
            tenCents += "cr:0.10;";
        tenCents.pop_back();
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD, statement("0.10", "0.30", "cr:0.20")).empty());
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD, statement("0", "1.00", tenCents)).empty());
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD,
            statement("90071992547409.93", "90071992547409.94", "cr:0.01")).empty());
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD,
            statement("0.30", "0.10", "dr:0.10;dr:0.10")).empty());
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD, statement("0.10", "0.31", "cr:0.20")).size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(STATEMENT_XSD,
            statement("90071992547409.93", "90071992547409.93", "cr:0.01")).size() == 1);
    }
}

namespace suites
{
    // Регистрация тестов утверждений на примерах пользователя
    // Registers the assertion tests on the user's examples
    void registerXsdAssertTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: xs:assert, пример пользователя", testUserExample);
        runner.add("XSD 1.1: xs:assert, нарушения баланса", testBalanceViolations);
        runner.add("XSD 1.1: xs:assert, точная десятичная арифметика", testExactCents);
    }
}
