#include "content/content_cursor.hpp"

namespace beresta::content
{
    // Пустой курсор; модель задаётся через reset()
    // An empty cursor; the model is set through reset()
    ContentCursor::ContentCursor()
        : _model(nullptr), _plan(CursorPlan::GREEDY_SEQUENCE), _failed(false), _consumed(0), _leaf(0),
          _taken(0), _current(), _next(), _work(), _buffered(), _bufferedCount(0)
    {
    }

    // Состояние прошлого элемента сбрасывается, а память буферов остаётся для этого
    // The state of the previous element is reset, and the buffer memory stays for this one
    void ContentCursor::reset(const Particle& model, CursorPlan plan)
    {
        _model = &model;
        _plan = plan;
        _failed = false;
        _consumed = 0;
        _leaf = 0;
        _taken = 0;
        _current.clear();
        _bufferedCount = 0;
    }

    /*
    ** Шаг по одному ребёнку. После первой неудачи курсор «замирает»: ошибка
    ** уже найдена на этом ребёнке, и последующие дети её не меняют (так же
    ** общий алгоритм сообщает лишь самую дальнюю достигнутую позицию).
    **
    ** A step over one child. After the first failure the cursor "freezes": the error
    ** has already been found at this child, and the following children do not change it (the
    ** general algorithm likewise reports only the farthest reached position).
    */
    bool ContentCursor::step(const ChildKey& child)
    {
        bool accepted = true;

        if (_failed)
            return false;
        if (_plan == CursorPlan::GREEDY_SEQUENCE)
            accepted = stepGreedy(child);
        else if (_plan == CursorPlan::AUTOMATON)
            accepted = stepAutomaton(child);
        else
            stepBuffered(child);
        if (!accepted)
        {
            _failed = true;
            return false;
        }
        ++_consumed;
        return true;
    }

    // Итог: при сбое — индекс ребёнка-нарушителя, иначе проверка, можно ли закончить модель здесь
    // The result: on a failure — the index of the offending child, otherwise a check whether the model can end here
    MatchResult ContentCursor::finish()
    {
        bool complete = false;

        if (_failed)
            return MatchResult{false, _consumed};
        if (_plan == CursorPlan::BUFFERED)
            return finishBuffered();
        if (_plan == CursorPlan::GREEDY_SEQUENCE)
            complete = finishGreedy();
        else
            complete = finishAutomaton();
        if (complete)
            return MatchResult{true, 0};
        return MatchResult{false, _consumed};
    }

    /*
    ** У жадного прохода лист — текущая частица последовательности, у
    ** автомата — вершина стека конфигурации (descend кладёт её последней).
    **
    ** For the greedy pass the leaf is the current particle of the sequence, for
    ** the automaton it is the top of the configuration stack (descend pushes it last).
    */
    const Particle* ContentCursor::matchedLeaf() const noexcept
    {
        if (_failed || _consumed == 0 || _model == nullptr)
            return nullptr;
        if (_plan == CursorPlan::GREEDY_SEQUENCE && _leaf < _model->children.size())
            return &_model->children[_leaf];
        if (_plan != CursorPlan::AUTOMATON || _current.count == 0 || _current.items[0].empty())
            return nullptr;
        return _current.items[0].back().particle;
    }
}
