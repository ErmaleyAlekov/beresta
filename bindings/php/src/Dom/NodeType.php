<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Типы узлов дерева (порядок совпадает с beresta_node_kind).
 * Node types of the tree (the order matches beresta_node_kind).
 */
enum NodeType: int
{
    case Document = 0;
    case DocumentType = 1;
    case Element = 2;
    case Text = 3;
    case CdataSection = 4;
    case Comment = 5;
    case ProcessingInstruction = 6;
}
