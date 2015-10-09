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
#include "core/editing/commands/OutdentBlockCommand.h"

#include "core/dom/Document.h"
#include "core/dom/NodeTraversal.h"
#include "core/editing/EditingUtilities.h"
#include "core/html/HTMLElement.h"
#include "core/HTMLNames.h"

namespace blink {

using namespace HTMLNames;

static bool hasListAncestor(Node* node, Node* stayWithin)
{
    node = node->parentNode();
    while (node != stayWithin) {
        if (isHTMLListElement(node))
            return true;
        node = node->parentNode();
    }
    return false;
}

static bool isIndentationBlock(Node* node, Node* stayWithin)
{
    if (isHTMLListElement(node)) {
        return hasListAncestor(node, stayWithin);
    }
    return node->hasTagName(blockquoteTag);
}

static Node* findCommonIndentationBlock(Node* firstSibling, Node* lastSibling, Node* stayWithin)
{
    ASSERT(firstSibling->isDescendantOf(stayWithin));
    ASSERT(firstSibling->parentNode() == lastSibling->parentNode());

    Node* node = firstSibling == lastSibling ? firstSibling : firstSibling->parentNode();
    while (node != stayWithin) {
        if (isIndentationBlock(node, stayWithin))
            return node;
        node = node->parentNode();
    }
    return 0;
}

static bool hasVisibleChildren(Node* node)
{
    for (Node* child = node->firstChild(); child; child = child->nextSibling()) {
        if (child->layoutObject())
            return true;
    }
    return false;
}

OutdentBlockCommand::OutdentBlockCommand(Document& document)
    : BlockCommand(document)
{
}

PassRefPtr<Node> OutdentBlockCommand::splitStart(Node* ancestor, PassRefPtr<Node> prpChild)
{
    ASSERT(prpChild->isDescendantOf(ancestor));

    RefPtr<Node> child = prpChild;

    while (child != ancestor) {
        RefPtr<Node> previous = previousRenderedSiblingExcludingWhitespace(child.get());
        if (previous)
            splitElement(toElement(child->parentNode()), previous->nextSibling());
        child = child->parentNode();
    }

    child = child->firstChild();
    return child.release();
}

PassRefPtr<Node> OutdentBlockCommand::splitEnd(Node* ancestor, PassRefPtr<Node> prpChild)
{
    ASSERT(prpChild->isDescendantOf(ancestor));

    RefPtr<Node> child = prpChild;
    bool reachedAncestor = false;

    while (!reachedAncestor) {
        reachedAncestor = child->parentNode() == ancestor;
        RefPtr<Node> next = nextRenderedSiblingExcludingWhitespace(child.get());
        if (next)
            splitElement(toElement(child->parentNode()), next);
        child = child->parentNode();
    }

    child = child->lastChild();
    return child.release();
}

void OutdentBlockCommand::outdentSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* indentBlock)
{
    ASSERT(indentBlock);
    if (!prpFirstSibling) {
        ASSERT(!prpLastSibling);
        ASSERT(!indentBlock->firstChild());
        removeNode(indentBlock);
        return;
    }

    ASSERT(prpFirstSibling->isDescendantOf(indentBlock));
    ASSERT(prpFirstSibling->parentNode() == prpLastSibling->parentNode());

    RefPtr<Node> firstSibling = prpFirstSibling;
    RefPtr<Node> lastSibling = prpLastSibling;

    lastSibling = splitEnd(indentBlock, lastSibling);
    indentBlock = lastSibling->parentNode();
    firstSibling = splitStart(indentBlock, firstSibling);
    ASSERT(firstSibling->parentNode() == indentBlock);

    RefPtr<Node> current = firstSibling;
    RefPtr<Node> end = lastSibling->nextSibling();
    while (current != end) {
        RefPtr<Node> next = current->nextSibling();
        removeNode(current);
        insertNodeBefore(current, indentBlock);
        current = next;
    }

    if (!hasVisibleChildren(indentBlock)) {
        removeNode(indentBlock);
    }
}

void OutdentBlockCommand::formatBlockSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* stayWithin, Node* lastNode)
{
    ASSERT(prpFirstSibling);
    ASSERT(prpLastSibling);
    ASSERT(prpFirstSibling->parentNode());
    ASSERT(prpFirstSibling->parentNode() == prpLastSibling->parentNode());
    ASSERT(prpFirstSibling->isDescendantOf(stayWithin));

    RefPtr<Node> firstSibling = prpFirstSibling;
    RefPtr<Node> lastSibling = prpLastSibling;

    Node* indentBlock = findCommonIndentationBlock(firstSibling.get(), lastSibling.get(), stayWithin);
    if (indentBlock) {
        if (indentBlock == firstSibling) {
            ASSERT(indentBlock == lastSibling);
            removeNodePreservingChildren(firstSibling);
        }
        else
            outdentSiblings(firstSibling, lastSibling, indentBlock);
        return;
    }

    // If there is no common indent block, then look to see if any of
    // the siblings, or their children, are themselves indent blocks,
    // and if so, remove them to remove the indentation
    RefPtr<Node> current = firstSibling;
    RefPtr<Node> end = NodeTraversal::nextSkippingChildren(*lastSibling, stayWithin);
    while (current != end) {
        RefPtr<Node> next;
        if (isIndentationBlock(current.get(), stayWithin)) {
            next = NodeTraversal::nextSkippingChildren(*current, stayWithin);
            removeNodePreservingChildren(current);
        }
        else
            next = NodeTraversal::next(*current, stayWithin);
        current = next;
    }
}

}
