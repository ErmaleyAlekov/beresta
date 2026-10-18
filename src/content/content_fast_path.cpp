#include "content/content_matcher.hpp"

namespace beresta::content
{
    // Листьев в последовательности не больше этого — иначе проверка имён попарно уже не дешёвая
    // No more leaves in a sequence than this — otherwise the pairwise name check is no longer cheap
    static const std::size_t FAST_PATH_LEAVES = 16;

    /*
    ** Подходит ли модель для жадного прохода: последовательность ровно
    ** один раз, все её частицы — элементы (не группы и не xs:any), и
    ** имена всех элементов попарно различны. Тогда каждый ребёнок может
    ** подойти только к одной частице, и выбора «где закончить повтор» нет.
    **
    ** Whether the model fits the greedy pass: a sequence exactly
    ** once, all its particles are elements (not groups and not xs:any), and
    ** the names of all elements are pairwise distinct. Then every child can
    ** fit only one particle, and there is no choice of "where to end a repetition".
    */
    static bool isSimpleSequence(const Particle& model) noexcept
    {
        const std::vector<Particle>& leaves = model.children;

        if (model.kind != ParticleKind::SEQUENCE || model.minOccurs != 1 || model.maxOccurs != 1
            || leaves.size() > FAST_PATH_LEAVES)
            return false;
        for (std::size_t index = 0; index < leaves.size(); ++index)
        {
            if (leaves[index].kind != ParticleKind::ELEMENT)
                return false;
            for (std::size_t other = 0; other < index; ++other)
            {
                if (leaves[other].name == leaves[index].name
                    && leaves[other].namespaceUri == leaves[index].namespaceUri)
                    return false;
            }
        }
        return true;
    }

    /*
    ** Быстрый путь для самой частой модели (последовательность элементов
    ** с разными именами): один проход по детям без множеств позиций и без
    ** выделения памяти. Каждая частица забирает подряд идущих детей со
    ** своим именем, не больше maxOccurs, и обязана набрать minOccurs.
    ** Возвращает true, только если модель подходит И дети ей соответствуют;
    ** при false вызывающий запускает общий алгоритм — он же найдёт точное
    ** место ошибки для сообщения.
    **
    ** The fast path for the most common model (a sequence of elements
    ** with different names): one pass over the children without position sets and without
    ** memory allocation. Each particle takes the consecutive children with
    ** its name, no more than maxOccurs, and must collect minOccurs.
    ** Returns true only if the model fits AND the children match it;
    ** on false the caller runs the general algorithm — it also finds the exact
    ** error location for the message.
    */
    bool matchesSimpleSequence(const Particle& model, const std::vector<ChildKey>& children)
    {
        std::size_t position = 0;

        if (!isSimpleSequence(model))
            return false;
        for (const Particle& leaf : model.children)
        {
            std::size_t count = 0;

            while (position < children.size() && (leaf.maxOccurs == UNBOUNDED || count < leaf.maxOccurs)
                && particleAccepts(leaf, children[position]))
            {
                ++position;
                ++count;
            }
            if (count < leaf.minOccurs)
                return false;
        }
        return position == children.size();
    }
}
