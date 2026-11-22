using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Нагрузочные проверки через границу P/Invoke.
    /// Load checks across the P/Invoke boundary.</summary>
    internal static class LoadChecks
    {
        private const int Nodes = 50000;

        /// <summary>
        /// 50 000 узлов переносятся из A в B с конца, A закрывается. Обёртки,
        /// выданные ДО переноса, — те же объекты (реестр обёрток), их
        /// документ — B, а XPath видит всё на новом месте.
        /// 50,000 nodes are moved from A to B from the end, A is closed. The wrappers
        /// given out BEFORE the move are the same objects (the wrapper registry),
        /// their document is B, and XPath sees everything in the new place.
        /// </summary>
        internal static void BulkMove()
        {
            StringBuilder xml = new StringBuilder("<a>");
            for (int index = 0; index < Nodes; ++index)
                xml.Append("<n>").Append(index).Append("</n>");
            XmlDocument source = Xml.Parse(xml.Append("</a>").ToString());
            using XmlDocument target = Xml.Parse("<b/>");
            using XPath count = new XPath("count(/b/n)");
            List<XmlNode> wrappers = source.Root!.Children.ToList();

            for (int index = Nodes - 1; index >= 0; --index)
                wrappers[index].MoveTo(target.Root!);
            source.Dispose();
            Expect.True(ReferenceEquals(wrappers[Nodes - 1], target.Root!.FirstChild), "та же обёртка первого");
            Expect.True(ReferenceEquals(target, wrappers[123].Document), "владелец сменился");
            Expect.Equal("123", wrappers[123].Text, "текст после закрытия A");
            Expect.Equal((double)Nodes, count.EvaluateNumber(target), "XPath видит все узлы");
        }

        /// <summary>
        /// 8 потоков x 40 запросов на одной схеме и одном XPath: все ответы одинаковы.
        /// 8 threads x 40 requests on one schema and one XPath: all answers are the same.
        /// </summary>
        internal static void ParallelShared()
        {
            StringBuilder xml = new StringBuilder("<order>");
            for (int index = 1; index <= 2000; ++index)
                xml.Append("<item qty='").Append(index % 1500).Append("'>").Append(index).Append("</item>");
            byte[] data = Encoding.UTF8.GetBytes(xml.Append("</order>").ToString());
            using XmlSchema schema = XmlSchema.FromString(StreamingChecks.OrderXsd);
            using XPath xpath = new XPath("concat(count(//item), ':', //item[2])");
            ConcurrentBag<string> answers = new ConcurrentBag<string>();
            const int workers = 8;
            using CountdownEvent ready = new CountdownEvent(workers);
            using ManualResetEventSlim start = new ManualResetEventSlim(false);
            ValidationWorker workload = new ValidationWorker(schema, xpath, data, answers);
            RunWorkers(workers, workload, ready, start);
            if (!workload.Failures.IsEmpty)
                throw new AggregateException(workload.Failures);
            Expect.Equal(320, answers.Count, "число ответов");
            Expect.Equal("1/2000:2", string.Join(",", answers.Distinct()), "одинаковые ответы");
        }
        /// <summary>Дожидается готовности всех потоков, затем запускает запросы одновременно.
        /// Waits until all threads are ready, then starts their requests together.</summary>
        private static void RunWorkers(int workers, ValidationWorker workload, CountdownEvent ready, ManualResetEventSlim start)
        {
            List<Thread> threads = new List<Thread>(workers);
            ThreadStart run = workload.StartTogether(ready, start);
            try
            {
                for (int worker = 0; worker < workers; ++worker)
                {
                    Thread thread = new Thread(run);
                    thread.Start();
                    threads.Add(thread);
                }
                ready.Wait();
            }
            finally
            {
                start.Set();
                foreach (Thread thread in threads)
                    thread.Join();
            }
        }
    }
}
