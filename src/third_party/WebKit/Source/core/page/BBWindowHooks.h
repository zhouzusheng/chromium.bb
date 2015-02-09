/*
 * Copyright (C) 2013 Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#ifndef BBWindowHooks_h
#define BBWindowHooks_h

#include "core/frame/DOMWindowProperty.h"
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>
#include <wtf/ArrayBuffer.h>

namespace blink {

    class LocalFrame;
    class Node;
    class Document;
    class Range;
    class ClientRect;

    class BBWindowHooks : public RefCounted<BBWindowHooks>, public DOMWindowProperty {
    public:
        static PassRefPtr<BBWindowHooks> create(LocalFrame *frame) { return adoptRef(new BBWindowHooks(frame)); }

        bool isBlock(Node* node);
        String getPlainText(Node* node, const String& excluder = "", const String& mask = "");
        void fakePaint(Document* document);
        void setTextMatchMarkerVisibility(Document* document, bool highlight);
        bool checkSpellingForRange(Range* range);
        void removeMarker(Range* range, long mask, long removeMarkerFlag);
        void addMarker(Range* range, long markerType);
        PassRefPtr<Range> findPlainText(Range* range, const String& target, long options);
        bool checkSpellingForNode(Node* node);
        PassRefPtr<ClientRect> getAbsoluteCaretRectAtOffset(Node* node, long offset);

    private:

        explicit BBWindowHooks(LocalFrame *frame);
        bool matchSelector(Node *node, const String& selector);
        void appendTextContent(Node *node, StringBuilder& content, const String& excluder, const String& mask);
    };

} // namespace blink

#endif // BBWindowHooks_h
