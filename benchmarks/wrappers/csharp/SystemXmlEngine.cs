using System.Collections.Generic;
using System.IO;
using System.Xml;

namespace WrapperBench
{
    /// <summary>
    /// System.Xml.XmlDocument — классический DOM .NET с SelectNodes и проверкой
    /// по схеме через XmlReader. Настройки чтения создаются один раз.
    ///
    /// System.Xml.XmlDocument — the classic .NET DOM with SelectNodes and schema
    /// validation through XmlReader. The reader settings are created once.
    /// </summary>
    public sealed class SystemXmlEngine : Engine
    {
        private readonly ErrorCounter validation;
        private readonly string query;

        /// <summary>Схема компилируется один раз, ошибки проверки считаются. / The schema is compiled once, validation errors are counted.</summary>
        public SystemXmlEngine(string xsd, string expression)
            : base("System.Xml XmlDocument")
        {
            query = expression;
            validation = new ErrorCounter(xsd);
        }

        public override object Parse(byte[] xml)
        {
            System.Xml.XmlDocument document = new System.Xml.XmlDocument();
            document.Load(new MemoryStream(xml));
            return document;
        }

        /// <summary>Тот же обход явным стеком, что и у «Берёсты». / The same explicit-stack walk as for Beresta.</summary>
        public override int Walk(object document)
        {
            Stack<System.Xml.XmlNode> pending = new Stack<System.Xml.XmlNode>();
            int count = 0;

            pending.Push((System.Xml.XmlNode)document);
            while (pending.Count > 0)
            {
                System.Xml.XmlNode node = pending.Pop();
                if (node.NodeType == XmlNodeType.Element)
                    ++count;
                for (System.Xml.XmlNode? child = node.FirstChild; child != null; child = child.NextSibling)
                    pending.Push(child);
            }
            return count;
        }

        public override int XPath(object document)
        {
            return ((System.Xml.XmlDocument)document).SelectNodes(query)!.Count;
        }

        /// <summary>Разбор в дерево через проверяющий XmlReader. / Parsing into a tree through a validating XmlReader.</summary>
        public override int ParseValidated(byte[] xml)
        {
            System.Xml.XmlDocument document = new System.Xml.XmlDocument();

            validation.Errors = 0;
            using (XmlReader reader = XmlReader.Create(new MemoryStream(xml), validation.Settings))
                document.Load(reader);
            return validation.Errors;
        }
    }
}
