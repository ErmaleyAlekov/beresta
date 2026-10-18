using System.Collections.Generic;
using Beresta;

namespace WrapperBench
{
    /// <summary>
    /// «Берёста» через обёртку P/Invoke: каждый шаг обхода и каждый узел
    /// результата XPath — вызов нативной функции и объект-обёртка .NET.
    ///
    /// Beresta through the P/Invoke wrapper: every walk step and every node
    /// of an XPath result is a native function call and a .NET wrapper object.
    /// </summary>
    public sealed class BerestaEngine : Engine
    {
        private readonly XmlSchema schema;
        private readonly Beresta.XPath query;

        /// <summary>Схема и запрос готовятся один раз. / The schema and the query are prepared once.</summary>
        public BerestaEngine(string xsd, string expression)
            : base("Берёста / Beresta " + BerestaLibrary.Version + " (P/Invoke)")
        {
            schema = XmlSchema.FromString(xsd);
            query = new Beresta.XPath(expression);
        }

        public override object Parse(byte[] xml)
        {
            return Xml.Parse(xml);
        }

        /// <summary>Обход явным стеком: FirstChild / NextSibling у каждого узла. / A walk with an explicit stack: FirstChild / NextSibling of every node.</summary>
        public override int Walk(object document)
        {
            Stack<XmlNode> pending = new Stack<XmlNode>();
            int count = 0;

            pending.Push((XmlNode)document);
            while (pending.Count > 0)
            {
                XmlNode node = pending.Pop();
                if (node.Type == NodeType.Element)
                    ++count;
                for (XmlNode? child = node.FirstChild; child != null; child = child.NextSibling)
                    pending.Push(child);
            }
            return count;
        }

        public override int XPath(object document)
        {
            return query.SelectNodes((XmlNode)document).Count;
        }

        /// <summary>Проверка идёт во время разбора; ошибка проверки — исключение. / Validation runs during parsing; a validation error is an exception.</summary>
        public override int ParseValidated(byte[] xml)
        {
            using (XmlDocument document = Xml.Parse(xml, ParseFlags.Default, schema))
                return 0;
        }
    }
}
