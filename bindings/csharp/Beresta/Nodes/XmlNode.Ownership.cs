namespace Beresta
{
    /// <summary>Смена документа-владельца при переносе узла.
    /// Changing the owner document when a node is moved.</summary>
    public partial class XmlNode
    {
        /// <summary>
        /// Узел теперь принадлежит документу target: проверка «документ закрыт»
        /// и сильная ссылка, не дающая собрать документ (а с ним SafeHandle),
        /// переходят к нему. Прежний документ можно закрывать — узла это не коснётся.
        /// The node now belongs to the target document: the "document is closed" check
        /// and the strong reference that keeps the document (and its SafeHandle) from
        /// being collected move to it. The former document can be closed — it will not
        /// touch the node.
        /// </summary>
        internal void Adopt(XmlDocument target)
        {
            owner = target;
        }
    }
}
