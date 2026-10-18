using System.IO;
using System.Xml;
using System.Xml.Linq;
using System.Xml.XPath;

namespace WrapperBench
{
    /// <summary>
    /// LINQ to XML (XDocument) — современный API .NET: Descendants для обхода,
    /// XPathSelectElements для XPath и проверяющий XmlReader для схемы.
    ///
    /// LINQ to XML (XDocument) — the modern .NET API: Descendants for walking,
    /// XPathSelectElements for XPath and a validating XmlReader for the schema.
    /// </summary>
    public sealed class LinqXmlEngine : Engine
    {
        private readonly ErrorCounter validation;
        private readonly string query;

        /// <summary>Схема компилируется один раз, ошибки проверки считаются. / The schema is compiled once, validation errors are counted.</summary>
        public LinqXmlEngine(string xsd, string expression)
            : base("LINQ to XML XDocument")
        {
            query = expression;
            validation = new ErrorCounter(xsd);
        }

        public override object Parse(byte[] xml)
        {
            return XDocument.Load(new MemoryStream(xml));
        }

        /// <summary>Обход всех элементов через Descendants. / Walking all elements through Descendants.</summary>
        public override int Walk(object document)
        {
            int count = 0;

            foreach (XElement element in ((XDocument)document).Descendants())
                ++count;
            return count;
        }

        public override int XPath(object document)
        {
            int count = 0;

            foreach (XElement element in ((XDocument)document).XPathSelectElements(query))
                ++count;
            return count;
        }

        /// <summary>Разбор в дерево через проверяющий XmlReader. / Parsing into a tree through a validating XmlReader.</summary>
        public override int ParseValidated(byte[] xml)
        {
            validation.Errors = 0;
            using (XmlReader reader = XmlReader.Create(new MemoryStream(xml), validation.Settings))
                XDocument.Load(reader);
            return validation.Errors;
        }
    }
}
