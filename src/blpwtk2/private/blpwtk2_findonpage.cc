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

#include <blpwtk2_findonpage.h>

#include <content/public/browser/render_view_host.h>
#include <third_party/WebKit/public/web/WebFindOptions.h>

namespace blpwtk2 {

void FindOnPage::Request::executeOn(content::RenderViewHost* host) const
{
    if (!d_reqId) {
        host->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
        return;
    }
    WebKit::WebFindOptions options;
    options.findNext = d_findNext;
    options.forward = d_forward;
    options.matchCase = d_matchCase;
    WebKit::WebString textStr = toWebString(d_text);
    host->Find(d_reqId, textStr, options);
}

FindOnPage::Request FindOnPage::makeRequest(const StringRef& text,
                                            bool matchCase,
                                            bool forward)
{
    bool findNext = d_text.equals(text);
    if (!findNext) {
        d_text.assign(text);

        // clear values for a new find
        d_numberOfMatches = 0;
        d_activeMatchOrdinal = 0;

        if (!text.isEmpty() && ++d_reqId <= 0) {
            d_reqId = 1; // handle overflow
        }
    }
    int reqId = d_text.isEmpty() ? 0 : d_reqId; // 0 reqId to reset search
    return Request(reqId, d_text, matchCase, findNext, forward);
}

bool FindOnPage::applyUpdate(int reqId,
                             int numberOfMatches,
                             int activeMatchOrdinal)
{
    if (reqId != d_reqId) return false;

    if (numberOfMatches != -1) {
        d_numberOfMatches = numberOfMatches;
    }
    if (activeMatchOrdinal != -1) {
        d_activeMatchOrdinal = activeMatchOrdinal;
    }
    return true;
}

}  // close namespace blpwtk2
