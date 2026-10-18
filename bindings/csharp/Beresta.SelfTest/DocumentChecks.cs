using System;
using System.IO;
using System.Linq;
using System.Text;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Проверки документов и узлов.
    /// Document and node checks.</summary>
    internal static class DocumentChecks
    {
        /// <summary>Разбор и навигация: дети, соседи, родители, типы узлов.
        /// Parsing and navigation: children, siblings, parents, node types.</summary>
        internal static void ParseAndNavigate()
        {
            using XmlDocument document = Xml.Parse("<shop><item code='A1'>Чай</item><!--x--><item>Кофе</item></shop>");
            XmlElement root = document.Root!;

            Expect.Equal("shop", root.Name, "имя корня");
            Expect.Equal(3L, root.ChildCount, "число детей");
            Expect.Equal("A1", ((XmlElement)root.FirstChild!).GetAttribute("code"), "атрибут");
            Expect.Equal("Кофе", root.LastChild!.Text, "текст");
            Expect.Equal(NodeType.Comment, root.Child(1).Type, "тип комментария");
            Expect.True(ReferenceEquals(root.FirstChild!.NextSibling!.Parent!.Parent, document), "родитель корня — документ");
            Expect.Equal(3, root.Children.Count(), "перечисление детей");
        }

        /// <summary>Построение документа с нуля и сериализация.
        /// Building a document from scratch and serialization.</summary>
        internal static void BuildAndSerialize()
        {
            using XmlDocument document = Xml.NewDocument("order");
            XmlElement item = document.Root!.AppendElement("item");

            item.SetAttribute("qty", "2");
            item.AppendText("Мёд & орехи");
            Expect.Equal("<order><item qty=\"2\">Мёд &amp; орехи</item></order>", document.ToXml(), "компактный вывод");
            Expect.True(document.ToXml(WriteFlags.Pretty).Contains("\n    <item"), "вывод с отступами");
            Expect.Equal("qty", item.Attributes[0].Key, "перебор атрибутов");
            Expect.True(item.RemoveAttribute("qty") && !item.HasAttribute("qty"), "удаление атрибута");
        }

        /// <summary>Копирование между документами, удаление, глубокая копия документа.
        /// Copying between documents, removal, a deep copy of a document.</summary>
        internal static void CopyAndRemove()
        {
            using XmlDocument source = Xml.Parse("<a><b x='1'/></a>");
            using XmlDocument target = Xml.Parse("<c/>");
            XmlNode b = source.Root!.FirstChild!;

            b.CopyTo(target.Root!);
            b.Remove();
            using XmlDocument copy = target.Copy();
            Expect.Equal("<a/>", source.ToXml(), "после удаления");
            Expect.Equal("<c><b x=\"1\"/></c>", copy.ToXml(), "копия документа");
        }

        /// <summary>Ошибки: синтаксис, обращение к освобождённому документу.
        /// Errors: syntax, accessing a disposed document.</summary>
        internal static void Errors()
        {
            try
            {
                Xml.Parse("<a><b></a>");
                throw Expect.MissingError("синтаксическая ошибка");
            }
            catch (BerestaException error)
            {
                Expect.Equal("PARSE_ERROR", error.CodeName, "код ошибки разбора");
                Expect.True(error.Detail.StartsWith("строка 1", StringComparison.Ordinal), "текст ошибки: " + error.Detail);
            }
            XmlDocument document = Xml.Parse("<a/>");
            XmlElement root = document.Root!;
            document.Dispose();
            try
            {
                _ = root.Text;
                throw Expect.MissingError("освобождённый документ");
            }
            catch (ObjectDisposedException)
            {
                Expect.True(document.IsClosed, "документ закрыт");
            }
        }

        /// <summary>Запись в файл в другой кодировке и обратный разбор.
        /// Writing to a file in another encoding and parsing it back.</summary>
        internal static void FilesAndEncodings()
        {
            string folder = Directory.CreateDirectory(Path.Combine(Path.GetTempPath(), "beresta-cs-" + Guid.NewGuid())).FullName;
            string file = Path.Combine(folder, "документ.xml");

            using (XmlDocument document = Xml.Parse("<текст>Привет</текст>"))
                document.Save(file, WriteFlags.Default, "windows-1251");
            Expect.True(Encoding.ASCII.GetString(File.ReadAllBytes(file)).Contains("windows-1251"), "декларация кодировки");
            using (XmlDocument loaded = Xml.ParseFile(file))
                Expect.Equal("Привет", loaded.Root!.Text, "текст после перекодирования");
            Directory.Delete(folder, true);
        }
    }
}
