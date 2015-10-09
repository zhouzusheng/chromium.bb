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
#include "core/editing/commands/IndentBlockCommand.h"

#include "core/dom/Document.h"
#include "core/editing/EditingUtilities.h"
#include "core/html/HTMLElement.h"
#include "core/HTMLNames.h"

namespace blink {

using namespace HTMLNames;

static const HTMLQualifiedName& getBlockQuoteName(const Node* parent)
{
    if (isHTMLUListElement(parent))
        return ulTag;
    else if (isHTMLOListElement(parent))
        return olTag;
    else
        return blockquoteTag;
}

IndentBlockCommand::IndentBlockCommand(Document& document)
    : BlockCommand(document)
{
}

PassRefPtr<Element> IndentBlockCommand::createIndentBlock(const QualifiedName& tagName) const
{
    RefPtr<Element> element = createHTMLElement(document(), tagName);
    if (tagName.matches(blockquoteTag))
        element->setAttribute(styleAttr, "margin: 0 0 0 40px; border: none; padding: 0px;");
    return element.release();
}

void IndentBlockCommand::indentSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* lastNode)
{
    RefPtr<Node> firstSibling = prpFirstSibling;
    RefPtr<Node> lastSibling = prpLastSibling;

    RefPtr<Element> blockForIndent;
    RefPtr<Node> refChild;
    bool needToMergeNextSibling = false;

    const blink::HTMLQualifiedName blockQName
        = getBlockQuoteName(firstSibling->parentNode());

    RefPtr<Node> previousSibling = previousRenderedSiblingExcludingWhitespace(firstSibling.get());
    if (previousSibling && previousSibling->isElementNode()
            && toElement(previousSibling)->hasTagName(blockQName)) {
        blockForIndent = toElement(previousSibling.get());
        firstSibling = previousSibling->nextSibling();
    }

    RefPtr<Node> nextSibling = nextRenderedSiblingExcludingWhitespace(lastSibling.get());
    if (nextSibling && nextSibling->hasTagName(blockQName) && !lastNode->isDescendantOf(nextSibling.get())) {
        if (!blockForIndent) {
            blockForIndent = toElement(nextSibling.get());
            refChild = nextSibling->firstChild();
        }
        else if (nextSibling->firstChild())
            needToMergeNextSibling = true;
        lastSibling = nextSibling->previousSibling();
    }

    if (!blockForIndent) {
        blockForIndent = createIndentBlock(blockQName);
        insertNodeBefore(blockForIndent, firstSibling);
    }

    moveRemainingSiblingsToNewParent(firstSibling.get(), lastSibling->nextSibling(), blockForIndent, refChild);
    if (needToMergeNextSibling) {
        moveRemainingSiblingsToNewParent(nextSibling->firstChild(), nextSibling->lastChild()->nextSibling(), blockForIndent);
        removeNode(nextSibling);
    }
}

}
