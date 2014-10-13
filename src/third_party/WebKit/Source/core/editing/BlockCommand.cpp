/*
 * Copyright (C) 2013 Bloomberg L.P. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "core/editing/BlockCommand.h"

#include "core/dom/Document.h"
#include "core/dom/NodeTraversal.h"
#include "core/editing/htmlediting.h"
#include "core/html/HTMLElement.h"
#include "HTMLNames.h"

namespace WebCore {

using namespace HTMLNames;

static bool isTableCellOrRootEditable(const Node* node)
{
    return isTableCell(node) || (node && node->isRootEditableElement());
}

BlockCommand::BlockCommand(Document& document)
    : CompositeEditCommand(document)
{
}

void BlockCommand::formatBlockExtent(PassRefPtr<Node> prpFirstNode, PassRefPtr<Node> prpLastNode, Node* stayWithin)
{
    RefPtr<Node> currentNode = prpFirstNode;
    RefPtr<Node> endNode = prpLastNode;

    while (currentNode->isDescendantOf(endNode.get()))
        endNode = endNode->lastChild();

    while (currentNode) {
        while (endNode->isDescendantOf(currentNode.get())) {
            ASSERT(currentNode->firstChild());
            currentNode = currentNode->firstChild();
        }

        RefPtr<Node> firstSibling = currentNode;
        RefPtr<Node> lastSibling = currentNode;

        while (lastSibling != endNode) {
            RefPtr<Node> nextSibling = lastSibling->nextSibling();
            if (!nextSibling || endNode->isDescendantOf(nextSibling.get()))
                break;
            lastSibling = nextSibling;
        }

        RefPtr<Node> nextNode = lastSibling == endNode ? 0 : NodeTraversal::nextSkippingChildren(*lastSibling, stayWithin);
        formatBlockSiblings(firstSibling, lastSibling, stayWithin, endNode.get());
        currentNode = nextNode;
    }
}

void BlockCommand::formatBlockSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* stayWithin, Node* lastNode)
{
    ASSERT_NOT_REACHED();
}

void BlockCommand::doApply()
{
    VisiblePosition startOfSelection;
    VisiblePosition endOfSelection;
    RefPtr<ContainerNode> startScope;
    RefPtr<ContainerNode> endScope;
    int startIndex;
    int endIndex;

    if (!prepareForBlockCommand(startOfSelection, endOfSelection, startScope, endScope, startIndex, endIndex, true))
        return;
    formatSelection(startOfSelection, endOfSelection);
    finishBlockCommand(startScope, endScope, startIndex, endIndex);
}

void BlockCommand::formatSelection(const VisiblePosition& startOfSelection, const VisiblePosition& endOfSelection)
{
    // might be null if the recursion below goes awry
    if (startOfSelection.isNull() || endOfSelection.isNull())
        return;

    Node* startEnclosingCell = enclosingNodeOfType(startOfSelection.deepEquivalent(), &isTableCell);
    Node* endEnclosingCell = enclosingNodeOfType(endOfSelection.deepEquivalent(), &isTableCell);

    if (startEnclosingCell != endEnclosingCell) {
        if (startEnclosingCell && (!endEnclosingCell || !endEnclosingCell->isDescendantOf(startEnclosingCell))) {
            VisiblePosition newEnd = lastPositionInNode(startEnclosingCell);
            VisiblePosition nextStart = newEnd.next();
            while (isRenderedTableElement(nextStart.deepEquivalent().anchorNode()))
                nextStart = nextStart.next();
            // TODO: fix recursion!
            formatSelection(startOfSelection, newEnd);
            formatSelection(nextStart, endOfSelection);
            return;
        }

        ASSERT(endEnclosingCell);

        VisiblePosition nextStart = firstPositionInNode(endEnclosingCell);
        VisiblePosition newEnd = nextStart.previous();
        while (isRenderedTableElement(newEnd.deepEquivalent().anchorNode()))
            newEnd = newEnd.previous();
        // TODO: fix recursion!
        formatSelection(startOfSelection, newEnd);
        formatSelection(nextStart, endOfSelection);
        return;
    }


    Node* root = enclosingNodeOfType(startOfSelection.deepEquivalent(), &isTableCellOrRootEditable);
    if (!root || root == startOfSelection.deepEquivalent().anchorNode())
        return;

    RefPtr<Node> currentNode = blockExtentStart(startOfSelection.deepEquivalent().anchorNode(), root);
    RefPtr<Node> endNode = blockExtentEnd(endOfSelection.deepEquivalent().anchorNode(), root);

    while (currentNode->isDescendantOf(endNode.get()))
        endNode = endNode->lastChild();

    formatBlockExtent(currentNode, endNode, root);
}

}
