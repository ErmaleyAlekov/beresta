using System;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Проверки переноса узлов между документами (MoveTo).
    /// Checks of moving nodes between documents (MoveTo).</summary>
    internal static class MoveChecks
    {
        /// <summary>
        /// Главный сценарий: узел переносится из A в B, A закрывается, узлом
        /// и его потомком (обёртка которого получена ДО переноса) пользуются в B.
        /// The main scenario: a node is moved from A to B, A is closed, the node and
        /// its descendant (whose wrapper was obtained BEFORE the move) are used in B.
        /// </summary>
        internal static void MoveBetweenDocuments()
        {
            XmlDocument source = Xml.Parse("<a><item n='1'><name>Мёд</name></item><rest/></a>");
            using XmlDocument target = Xml.Parse("<b/>");
            XmlElement item = (XmlElement)source.Root!.FirstChild!;
            XmlNode name = item.FirstChild!;

            Expect.True(ReferenceEquals(item, source.Root!.FirstChild), "у узла одна обёртка");
            item.MoveTo(target.Root!);
            Expect.Equal("<a><rest/></a>", source.ToXml(), "A после переноса");
            source.Dispose();
            Expect.True(ReferenceEquals(target, item.Document), "владелец сменился");
            Expect.True(ReferenceEquals(target, name.Document), "владелец потомка сменился");
            Expect.Equal("Мёд", name.Text, "потомок после закрытия A");
            item.SetAttribute("n", "2");
            Expect.True(ReferenceEquals(target.Root, item.Parent), "родитель — корень B");
            Expect.Equal("<b><item n=\"2\"><name>Мёд</name></item></b>", target.ToXml(), "B после переноса");
        }

        /// <summary>
        /// Обратная сторона: после закрытия B перенесённый узел недоступен,
        /// хотя A ещё открыт, — понятное исключение вместо падения процесса.
        /// The other side: after B is closed the moved node is unavailable although
        /// A is still open — a clear exception instead of a process crash.
        /// </summary>
        internal static void ClosedTargetDocument()
        {
            using XmlDocument source = Xml.Parse("<a><item><name/></item></a>");
            XmlDocument target = Xml.Parse("<b/>");
            XmlNode item = source.Root!.FirstChild!;
            XmlNode name = item.FirstChild!;

            item.MoveTo(target.Root!);
            target.Dispose();
            try
            {
                _ = name.Name;
                throw Expect.MissingError("закрытый документ B");
            }
            catch (ObjectDisposedException)
            {
                Expect.Equal("<a/>", source.ToXml(), "A без перенесённого узла");
            }
        }

        /// <summary>Запрещённые переносы: корень, документ, цикл; дерево не меняется.
        /// Forbidden moves: the root, a document, a cycle; the tree does not change.</summary>
        internal static void MoveErrors()
        {
            using XmlDocument document = Xml.Parse("<r><p><q/></p></r>");
            XmlNode p = document.Root!.FirstChild!;

            ExpectCode("HIERARCHY_VIOLATION", document.Root!, p, "перенос корня");
            ExpectCode("HIERARCHY_VIOLATION", p, p.FirstChild!, "перенос в потомка");
            ExpectCode("INVALID_ARGUMENT", document, p, "перенос документа");
            Expect.Equal("<r><p><q/></p></r>", document.ToXml(), "дерево не изменилось");
            Expect.True(ReferenceEquals(document, p.Document), "владелец не изменился");
        }

        /// <summary>Перенос, который обязан завершиться ошибкой с кодом code.
        /// A move that must fail with the code code.</summary>
        private static void ExpectCode(string code, XmlNode node, XmlNode parent, string what)
        {
            try
            {
                node.MoveTo(parent);
                throw Expect.MissingError(what);
            }
            catch (BerestaException error)
            {
                Expect.Equal(code, error.CodeName, what);
            }
        }
    }
}
