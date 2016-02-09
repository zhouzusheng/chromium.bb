// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/savable_resources.h"

#include <set>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "content/renderer/web_frame_utils.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebElement.h"
#include "third_party/WebKit/public/web/WebElementCollection.h"
#include "third_party/WebKit/public/web/WebInputElement.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebNode.h"
#include "third_party/WebKit/public/web/WebView.h"

using blink::WebDocument;
using blink::WebElement;
using blink::WebElementCollection;
using blink::WebFrame;
using blink::WebInputElement;
using blink::WebLocalFrame;
using blink::WebNode;
using blink::WebString;
using blink::WebVector;
using blink::WebView;

namespace content {
namespace {

// Get all savable resource links from current element. One element might
// have more than one resource link. It is possible to have some links
// in one CSS stylesheet.
void GetSavableResourceLinkForElement(
    const WebElement& element,
    const WebDocument& current_doc,
    SavableResourcesResult* result) {
  if (element.hasHTMLTagName("iframe") || element.hasHTMLTagName("frame")) {
    GURL complete_url = current_doc.completeURL(element.getAttribute("src"));
    WebFrame* web_frame = WebFrame::fromFrameOwnerElement(element);

    SavableSubframe subframe;
    subframe.original_url = complete_url;
    subframe.routing_id = GetRoutingIdForFrameOrProxy(web_frame);

    result->subframes->push_back(subframe);
    return;
  }

  // Check whether the node has sub resource URL or not.
  WebString value = GetSubResourceLinkFromElement(element);
  if (value.isNull())
    return;
  // Get absolute URL.
  GURL u = current_doc.completeURL(value);
  // ignore invalid URL
  if (!u.is_valid())
    return;
  // Ignore those URLs which are not standard protocols. Because FTP
  // protocol does no have cache mechanism, we will skip all
  // sub-resources if they use FTP protocol.
  if (!u.SchemeIsHTTPOrHTTPS() && !u.SchemeIs(url::kFileScheme))
    return;

  result->resources_list->push_back(u);
}

}  // namespace

bool GetSavableResourceLinksForFrame(WebFrame* current_frame,
                                     SavableResourcesResult* result,
                                     const char** savable_schemes) {
  // Get current frame's URL.
  GURL current_frame_url = current_frame->document().url();

  // If url of current frame is invalid, ignore it.
  if (!current_frame_url.is_valid())
    return false;

  // If url of current frame is not a savable protocol, ignore it.
  bool is_valid_protocol = false;
  for (int i = 0; savable_schemes[i] != NULL; ++i) {
    if (current_frame_url.SchemeIs(savable_schemes[i])) {
      is_valid_protocol = true;
      break;
    }
  }
  if (!is_valid_protocol)
    return false;

  // Get current using document.
  WebDocument current_doc = current_frame->document();
  // Go through all descent nodes.
  WebElementCollection all = current_doc.all();
  // Go through all elements in this frame.
  for (WebElement element = all.firstItem(); !element.isNull();
       element = all.nextItem()) {
    GetSavableResourceLinkForElement(element,
                                     current_doc,
                                     result);
  }

  return true;
}

WebString GetSubResourceLinkFromElement(const WebElement& element) {
  const char* attribute_name = NULL;
  if (element.hasHTMLTagName("img") ||
      element.hasHTMLTagName("script")) {
    attribute_name = "src";
  } else if (element.hasHTMLTagName("input")) {
    const WebInputElement input = element.toConst<WebInputElement>();
    if (input.isImageButton()) {
      attribute_name = "src";
    }
  } else if (element.hasHTMLTagName("body") ||
             element.hasHTMLTagName("table") ||
             element.hasHTMLTagName("tr") ||
             element.hasHTMLTagName("td")) {
    attribute_name = "background";
  } else if (element.hasHTMLTagName("blockquote") ||
             element.hasHTMLTagName("q") ||
             element.hasHTMLTagName("del") ||
             element.hasHTMLTagName("ins")) {
    attribute_name = "cite";
  } else if (element.hasHTMLTagName("link")) {
    // If the link element is not linked to css, ignore it.
    if (base::LowerCaseEqualsASCII(
            base::StringPiece16(element.getAttribute("type")), "text/css") ||
        base::LowerCaseEqualsASCII(
            base::StringPiece16(element.getAttribute("rel")), "stylesheet")) {
      // TODO(jnd): Add support for extracting links of sub-resources which
      // are inside style-sheet such as @import, url(), etc.
      // See bug: http://b/issue?id=1111667.
      attribute_name = "href";
    }
  }
  if (!attribute_name)
    return WebString();
  WebString value = element.getAttribute(WebString::fromUTF8(attribute_name));
  // If value has content and not start with "javascript:" then return it,
  // otherwise return NULL.
  if (!value.isNull() && !value.isEmpty() &&
      !base::StartsWith(value.utf8(), "javascript:",
                        base::CompareCase::INSENSITIVE_ASCII))
    return value;

  return WebString();
}

}  // namespace content
