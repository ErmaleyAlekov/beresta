'use strict';

const { native } = require('../internal/native.js');
const { OWNER, REGISTRY } = require('../internal/keys.js');

// Минимальный размер, с которого начинается чистка / The minimal size the sweep starts from
const MINIMAL_SWEEP_SIZE = 64;

/**
 * Реестр обёрток одного документа: нативный дескриптор узла → его
 * единственная обёртка (паттерн Identity Map). Зачем он нужен: при переносе
 * узла в другой документ (moveTo) у ВСЕХ уже выданных обёрток этого узла и
 * его потомков должен смениться документ-владелец (символ OWNER). Иначе
 * обёртка, полученная до переноса, проверяла бы «жив ли документ» по
 * старому документу и после закрытия нового обратилась бы к освобождённой
 * памяти — процесс упал бы.
 *
 * Реестр хранит WeakRef и не продлевает жизнь обёрткам; записи умерших
 * обёрток вычищаются, когда реестр вырастает вдвое. Запись об удалённом узле
 * может пережить его (дескриптор достанется новому узлу), поэтому вместе
 * с обёрткой хранится её вид — элемент или нет — и сверяется при поиске.
 *
 * The wrapper registry of one document: the native node handle → its only
 * wrapper (the Identity Map pattern). Why it is needed: when a node is moved into
 * another document (moveTo), ALL wrappers already handed out for this node and
 * its descendants must change their owner document (the OWNER symbol). Otherwise
 * a wrapper obtained before the move would check "is the document alive" against
 * the old document and, after the new one is closed, would access freed memory —
 * the process would crash.
 *
 * The registry keeps WeakRef and does not extend the life of wrappers; the entries
 * of dead wrappers are swept out when the registry doubles. The entry of a removed
 * node may outlive it (the handle goes to a new node), so the wrapper kind — an
 * element or not — is kept with the wrapper and compared on lookup.
 */
class NodeRegistry
{
    constructor()
    {
        this.wrappers = new Map();
        this.sweepAt = MINIMAL_SWEEP_SIZE;
    }

    /** Живая обёртка узла нужного вида или null. / A live wrapper of the node of the required kind or null. */
    find(handle, element)
    {
        const entry = this.wrappers.get(handle);

        if (entry === undefined || entry.element !== element)
            return null;
        const node = entry.ref.deref();
        if (node === undefined)
            return null;
        return node;
    }

    /** Запоминание обёртки; при достижении порога — чистка умерших записей
     *  (порог становится вдвое больше оставшегося, в среднем O(1) на вставку).
     *  Remembers a wrapper; on reaching the threshold the dead entries are swept
     *  (the threshold becomes twice the rest, O(1) per insertion on average). */
    store(handle, node, element)
    {
        this.wrappers.set(handle, { ref: new WeakRef(node), element });
        if (this.wrappers.size < this.sweepAt)
            return;
        for (const [key, entry] of this.wrappers)
        {
            if (entry.ref.deref() === undefined)
                this.wrappers.delete(key);
        }
        this.sweepAt = Math.max(MINIMAL_SWEEP_SIZE, this.wrappers.size * 2);
    }

    /** Передача обёрток перенесённого поддерева документу target (и самой
     *  обёртки moved). Узлы уже живут в target, поэтому обход нативного
     *  поддерева безопасен; обход итеративный (явный стек). Если в реестре нет
     *  никого, кроме корня поддерева, обходить потомков незачем.
     *  Hands the wrappers of the moved subtree over to the target document (and the
     *  moved wrapper itself). The nodes already live in target, so walking the native
     *  subtree is safe; the walk is iterative (an explicit stack). If the registry
     *  holds nobody but the subtree root, there is no need to walk the descendants. */
    transfer(moved, root, target)
    {
        const deep = this.wrappers.size > 1;
        const pending = [root];

        moved[OWNER] = target;
        while (pending.length > 0)
        {
            const current = pending.pop();
            const entry = this.wrappers.get(current);
            let node;
            if (entry !== undefined)
            {
                this.wrappers.delete(current);
                node = entry.ref.deref();
            }
            if (node !== undefined)
            {
                node[OWNER] = target;
                target[REGISTRY].store(current, node, entry.element);
            }
            let child = null;
            if (deep)
                child = native.node_first_child(current);
            while (child !== null)
            {
                pending.push(child);
                child = native.node_next_sibling(child);
            }
        }
    }
}

module.exports = { NodeRegistry };
