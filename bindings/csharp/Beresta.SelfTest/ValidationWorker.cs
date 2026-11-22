using System;
using System.Collections.Concurrent;
using System.Threading;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>
    /// Поток-обработчик для многопоточной проверки: Rounds «запросов» с
    /// общими схемой и XPath по своим документам; ответы складываются в
    /// потокобезопасную коллекцию. Отдельный класс с методом Run вместо
    /// лямбды — так требует стиль проекта.
    /// A handler thread for the multithreaded check: Rounds "requests" with
    /// the shared schema and XPath over own documents; the answers go into a
    /// thread-safe collection. A separate class with a Run method instead of a
    /// lambda — the project style requires it.
    /// </summary>
    internal sealed class ValidationWorker
    {
        private const int Rounds = 40;
        private readonly XmlSchema schema;
        private readonly XPath xpath;
        private readonly byte[] data;
        private readonly ConcurrentBag<string> answers;

        internal ConcurrentBag<Exception> Failures { get; } = new ConcurrentBag<Exception>();

        internal ValidationWorker(XmlSchema schema, XPath xpath, byte[] data, ConcurrentBag<string> answers)
        {
            this.schema = schema;
            this.xpath = xpath;
            this.data = data;
            this.answers = answers;
        }

        /// <summary>Возвращает точку входа с общим барьером старта.
        /// Returns the entry point with a shared start gate.</summary>
        internal ThreadStart StartTogether(CountdownEvent ready, ManualResetEventSlim start)
        {
            return Run;

            void Run()
            {
                ready.Signal();
                start.Wait();
                try
                {
                    Validate();
                }
                catch (Exception failure)
                {
                    Failures.Add(failure);
                }
            }
        }

        /// <summary>Проверки и запросы одного потока. / The checks and queries of one thread.</summary>
        private void Validate()
        {
            for (int round = 0; round < Rounds; ++round)
            {
                int problems = schema.Validate(data).Count;
                using XmlDocument document = Xml.Parse(data);
                answers.Add(problems + "/" + xpath.EvaluateString(document));
            }
        }
    }
}
