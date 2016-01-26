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

#ifndef OutdentBlockCommand_h
#define OutdentBlockCommand_h

#include "core/editing/commands/BlockCommand.h"

namespace blink {

class OutdentBlockCommand : public BlockCommand {
public:
    static PassRefPtr<OutdentBlockCommand> create(Document& document)
    {
        return adoptRef(new OutdentBlockCommand(document));
    }

    virtual bool preservesTypingStyle() const { return true; }

private:
    explicit OutdentBlockCommand(Document&);

    virtual EditAction editingAction() const override { return EditActionOutdent; }
    PassRefPtr<Node> splitStart(Node* ancestor, PassRefPtr<Node> prpChild);
    PassRefPtr<Node> splitEnd(Node* ancestor, PassRefPtr<Node> prpChild);
    void outdentSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* indentBlock);

    virtual void formatBlockSiblings(PassRefPtr<Node> prpFirstSibling, PassRefPtr<Node> prpLastSibling, Node* stayWithin, Node* lastNode) override;
};

} // namespace blink

#endif // OutdentBlockCommand_h
