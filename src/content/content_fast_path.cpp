#include "content/content_cursor.hpp"
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
    bool isGreedySequence(const Particle& model) noexcept
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
    ** Жадный шаг: текущая частица забирает ребёнка, пока не набрала
    ** maxOccurs; иначе, если minOccurs уже набран, ход переходит к
    ** следующей частице. Состояние — номер частицы и число взятых ею
    ** детей, без множеств позиций и без выделения памяти. Имена различны,
    ** поэтому этот детерминированный автомат точен: первый отвергнутый
    ** ребёнок — тот же, что нашёл бы общий алгоритм.
    **
    ** A greedy step: the current particle takes the child until it has collected
    ** maxOccurs; otherwise, if minOccurs has already been collected, the turn passes to the
    ** next particle. The state is the particle number and the number of children
    ** it has taken, without position sets and without memory allocation. The names are different,
    ** so this deterministic automaton is exact: the first rejected
    ** child is the same one the general algorithm would find.
    */
    bool ContentCursor::stepGreedy(const ChildKey& child)
    {
        const std::vector<Particle>& leaves = _model->children;

        while (_leaf < leaves.size())
        {
            const Particle& leaf = leaves[_leaf];
            bool room = leaf.maxOccurs == UNBOUNDED || _taken < leaf.maxOccurs;
            if (room && particleAccepts(leaf, child))
            {
                ++_taken;
                return true;
            }
            if (_taken < leaf.minOccurs)
                return false;
            ++_leaf;
            _taken = 0;
        }
        return false;
    }

    // Конец детей: текущая частица набрала minOccurs, а все следующие необязательны
    // The end of the children: the current particle has collected minOccurs, and all the next ones are optional
    bool ContentCursor::finishGreedy() const
    {
        const std::vector<Particle>& leaves = _model->children;

        for (std::size_t index = _leaf; index < leaves.size(); ++index)
        {
            std::size_t taken = 0;
            if (index == _leaf)
                taken = _taken;
            if (taken < leaves[index].minOccurs)
                return false;
        }
        return true;
    }

    /*
    ** Быстрый путь matchContent для готового списка детей (его использует
    ** валидатор DTD): тот же жадный курсор, пройденный по всему списку.
    ** false — модель не подходит или дети ей не соответствуют; тогда
    ** вызывающий запускает общий алгоритм, он же найдёт место ошибки.
    **
    ** The matchContent fast path for a ready child list (the DTD validator
    ** uses it): the same greedy cursor walked over the whole list.
    ** false — the model does not fit or the children do not match it; then
    ** the caller runs the general algorithm, which also finds the error location.
    */
    bool matchesSimpleSequence(const Particle& model, const std::vector<ChildKey>& children)
    {
        ContentCursor cursor;

        if (!isGreedySequence(model))
            return false;
        cursor.reset(model, CursorPlan::GREEDY_SEQUENCE);
        for (const ChildKey& child : children)
        {
            if (!cursor.step(child))
                return false;
        }
        return cursor.finish().matched;
    }
}
