using System.Collections.Generic;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Проверки XPath и XML Schema.
    /// XPath and XML Schema checks.</summary>
    internal static class XPathSchemaChecks
    {
        private const string Shop = "<s:shop xmlns:s='urn:s'><s:item code='A'>5</s:item><s:item code='B'>7</s:item></s:shop>";

        private const string SchemaText = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
            + "<xs:element name='n' type='xs:int'/></xs:schema>";

        /// <summary>Префиксы, переменные, число / логическое / строка.
        /// Prefixes, variables, number / boolean / string.</summary>
        internal static void XPathValues()
        {
            using XmlDocument document = Xml.Parse(Shop);
            using XPath sum = new XPath("sum(//p:item[@code != $skip])").Bind("p", "urn:s").Set("skip", "A");
            using XPath count = new XPath("count(//*) = 3");
            using XPath second = new XPath("string(//*[local-name()='item'][2])");

            Expect.Equal(7.0, sum.EvaluateNumber(document), "sum()");
            Expect.True(count.EvaluateBoolean(document), "boolean()");
            Expect.Equal("7", second.EvaluateString(document), "string()");
        }

        /// <summary>Выборка узлов, значения атрибутов, пространства имён, синтаксическая ошибка.
        /// Selecting nodes, attribute values, namespaces, a syntax error.</summary>
        internal static void XPathSelect()
        {
            using XmlDocument document = Xml.Parse(Shop);
            using XPath items = new XPath("//p:item").Bind("p", "urn:s");
            using XPath codes = new XPath("//@code");
            IReadOnlyList<XmlNode> found = items.SelectNodes(document);

            Expect.Equal(2, found.Count, "число узлов");
            Expect.Equal("urn:s", ((XmlElement)found[0]).NamespaceUri, "URI пространства имён");
            Expect.Equal("item", ((XmlElement)found[1]).LocalName, "локальное имя");
            Expect.Equal("A,B", string.Join(",", codes.SelectValues(document)), "значения атрибутов");
            try
            {
                using XPath broken = new XPath("//[");
                throw Expect.MissingError("ошибка XPath");
            }
            catch (BerestaException error)
            {
                Expect.Equal("XPATH_ERROR", error.CodeName, "код ошибки XPath");
            }
        }

        /// <summary>Проверка по схеме: список нарушений и проверка прямо при разборе.
        /// Schema validation: a list of violations and validation during parsing.</summary>
        internal static void Schema()
        {
            using XmlSchema schema = XmlSchema.FromString(SchemaText);
            using XmlDocument bad = Xml.Parse("<n>abc</n>");

            Expect.Equal(1, schema.Validate(bad).Count, "одно нарушение");
            using (XmlDocument good = Xml.Parse("<n>12</n>", ParseFlags.Default, schema))
                Expect.Equal("12", good.Root!.Text, "корректный документ");
            try
            {
                using XmlDocument invalid = Xml.Parse("<n>x</n>", ParseFlags.Default, schema);
                throw Expect.MissingError("ошибка валидации");
            }
            catch (BerestaException error)
            {
                Expect.Equal("VALIDATION_ERROR", error.CodeName, "код ошибки валидации");
            }
        }
    }
}
