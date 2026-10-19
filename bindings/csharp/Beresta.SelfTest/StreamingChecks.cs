using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Проверки потоковой проверки по схеме.
    /// Checks of streaming schema validation.</summary>
    internal static class StreamingChecks
    {
        internal const string OrderXsd = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
            + "<xs:element name='order'><xs:complexType><xs:sequence>"
            + "<xs:element name='item' maxOccurs='unbounded'><xs:complexType><xs:simpleContent>"
            + "<xs:extension base='xs:string'><xs:attribute name='qty' type='xs:positiveInteger' use='required'/>"
            + "</xs:extension></xs:simpleContent></xs:complexType></xs:element>"
            + "</xs:sequence></xs:complexType></xs:element></xs:schema>";

        internal const string OrderXml = "<order>\n<item qty='2'>чай</item>\n<item qty='0'>кофе</item>\n<item>мёд</item>\n</order>";

        /// <summary>
        /// Поток без дерева даёт те же нарушения, что и дерево с позициями
        /// узлов (TrackLocations); без флага места нарушений у дерева — 0.
        /// A stream without a tree gives the same violations as a tree with node
        /// positions (TrackLocations); without the flag the tree's violation places are 0.
        /// </summary>
        internal static void StreamMatchesTree()
        {
            using XmlSchema schema = XmlSchema.FromString(OrderXsd);
            using XmlDocument located = Xml.Parse(OrderXml, ParseFlags.TrackLocations);
            using XmlDocument plain = Xml.Parse(OrderXml);
            string stream = Describe(schema.Validate(Encoding.UTF8.GetBytes(OrderXml)));

            Expect.Equal(Describe(schema.Validate(located)), stream, "поток = дерево");
            Expect.True(stream.StartsWith("3:1: /order/item[2]", StringComparison.Ordinal), "место нарушения");
            Expect.Equal(0L, schema.Validate(plain)[0].Line, "без позиций — 0");
        }

        /// <summary>
        /// Файл с кириллическим именем, отсутствующий файл и синтаксическая ошибка.
        /// A file with a Cyrillic name, a missing file and a syntax error.
        /// </summary>
        internal static void StreamFile()
        {
            string directory = Directory.CreateTempSubdirectory("beresta_stream").FullName;
            string file = Path.Combine(directory, "выгрузка.xml");
            using XmlSchema schema = XmlSchema.FromString(OrderXsd);

            File.WriteAllText(file, OrderXml, new UTF8Encoding(false));
            try
            {
                Expect.Equal(Describe(schema.Validate(Encoding.UTF8.GetBytes(OrderXml))), Describe(schema.ValidateFile(file)), "файл = байты");
                Expect.Equal("IO_ERROR", CodeOf(schema, Path.Combine(directory, "нет.xml")), "нет файла");
                Expect.Equal("PARSE_ERROR", CodeOf(schema, null), "синтаксическая ошибка");
            }
            finally
            {
                Directory.Delete(directory, true);
            }
        }

        /// <summary>Код ошибки вызова: файл path или (null) заведомо битые байты.
        /// The call error code: the file path or (null) deliberately broken bytes.</summary>
        private static string CodeOf(XmlSchema schema, string? path)
        {
            try
            {
                if (path != null)
                    schema.ValidateFile(path);
                else
                    schema.Validate(Encoding.UTF8.GetBytes("<order><item qty='1'></order>"));
            }
            catch (BerestaException error)
            {
                return error.CodeName;
            }
            throw Expect.MissingError("ошибка вызова");
        }

        /// <summary>Список нарушений одной строкой «строка:столбец: текст».
        /// The list of violations as one "line:column: text" string.</summary>
        internal static string Describe(IReadOnlyList<Diagnostic> problems)
        {
            StringBuilder text = new StringBuilder();

            foreach (Diagnostic problem in problems)
                text.Append(problem.Line).Append(':').Append(problem.Column).Append(": ").Append(problem.Message).Append('\n');
            return text.ToString();
        }
    }
}
