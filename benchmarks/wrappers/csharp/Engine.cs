namespace WrapperBench
{
    /// <summary>
    /// Библиотека XML под замером (паттерн Strategy): разбор байтов в дерево,
    /// обход дерева, запрос XPath и разбор с проверкой по схеме. Документ,
    /// который реализует IDisposable, освобождает сама программа замеров.
    ///
    /// An XML library under measurement (the Strategy pattern): parsing bytes into a tree,
    /// walking the tree, an XPath query and parsing with schema validation. A document
    /// that implements IDisposable is freed by the benchmark program itself.
    /// </summary>
    public abstract class Engine
    {
        /// <summary>Имя для таблицы результатов. / The name for the results table.</summary>
        protected Engine(string name)
        {
            Name = name;
        }

        /// <summary>Имя библиотеки. / The library name.</summary>
        public string Name { get; }

        /// <summary>Разбор в дерево. / Parsing into a tree.</summary>
        public abstract object Parse(byte[] xml);

        /// <summary>Обход всего дерева; результат — число элементов. / Walking the whole tree; the result is the element count.</summary>
        public abstract int Walk(object document);

        /// <summary>Запрос XPath; результат — число найденных узлов. / An XPath query; the result is the found node count.</summary>
        public abstract int XPath(object document);

        /// <summary>Разбор в дерево с проверкой по схеме; результат — число ошибок. / Parsing into a tree with schema validation; the result is the error count.</summary>
        public abstract int ParseValidated(byte[] xml);
    }
}
