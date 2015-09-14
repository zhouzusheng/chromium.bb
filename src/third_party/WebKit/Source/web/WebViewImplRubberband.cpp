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

#include "config.h"
#include "WebViewImpl.h"

#include <vector>

#include "core/clipboard/Pasteboard.h"
#include "core/events/Event.h"
#include "core/frame/LocalFrame.h"
#include "core/frame/FrameView.h"
#include "core/layout/line/InlineTextBox.h"
#include "core/layout/LayoutBlock.h"
#include "core/layout/LayoutIFrame.h"
#include "core/layout/LayoutObject.h"
#include "core/layout/LayoutText.h"
#include "core/layout/LayoutView.h"
#include "core/layout/TextRunConstructor.h"
#include "core/page/Page.h"
#include "core/paint/DeprecatedPaintLayer.h"
#include "wtf/text/StringBuilder.h"

#include "public/web/WebViewClient.h"

namespace blink {

class RubberbandCandidate {
  public:
    WTF::String m_text;
    std::vector<LayoutUnit> m_charPositions;
    LayoutRect m_absRect;
    LayoutRect m_clipRect;
    LayoutUnit m_spaceWidth;
    int m_start;
    int m_len;
    bool m_isLTR;
    bool m_useLeadingTab;

    bool isAllWhitespaces() const
    {
        if (m_text.impl()) {
            int end = m_start + m_len;
            const StringImpl& impl = *m_text.impl();
            for (int i = m_start; i < end; ++i) {
                if (!isSpaceOrNewline(impl[i])) {
                    return false;
                }
            }
        }
        return true;
    }
};

struct RubberbandCandidate_XSorter {
    bool operator()(const RubberbandCandidate& lhs,
                    const RubberbandCandidate& rhs)
    {
        return lhs.m_clipRect.x() < rhs.m_clipRect.x();
    }
};

struct RubberbandCandidate_YSorter {
    bool operator()(const RubberbandCandidate& lhs,
                    const RubberbandCandidate& rhs)
    {
        return lhs.m_absRect.y() < rhs.m_absRect.y();
    }
};

class RubberbandStateImpl {
  public:
    std::vector<RubberbandCandidate> m_candidates;
    IntPoint m_startPoint;
};

RubberbandState::RubberbandState()
: m_impl(new RubberbandStateImpl())
{
}

RubberbandState::~RubberbandState()
{
    delete m_impl;
}

class RubberbandLayerContext {
  public:
    LayoutRect m_clipRect;
    FloatSize m_frameScrollOffset;
    FloatSize m_layerScrollOffset;
    double m_translateX;
    double m_translateY;
    double m_scaleX;
    double m_scaleY;
    LayoutBlock* m_colBlock;
    LayoutPoint m_colBlockAbsTopLeft;

    RubberbandLayerContext()
    : m_translateX(0.0)
    , m_translateY(0.0)
    , m_scaleX(1.0)
    , m_scaleY(1.0)
    , m_colBlock(0)
    {
    }

    RubberbandLayerContext(const RubberbandLayerContext* parent)
    : m_clipRect(parent->m_clipRect)
    , m_frameScrollOffset(parent->m_frameScrollOffset)
    , m_layerScrollOffset(parent->m_layerScrollOffset)
    , m_translateX(parent->m_translateX)
    , m_translateY(parent->m_translateY)
    , m_scaleX(parent->m_scaleX)
    , m_scaleY(parent->m_scaleY)
    , m_colBlock(parent->m_colBlock)
    , m_colBlockAbsTopLeft(parent->m_colBlockAbsTopLeft)
    {
    }
};

class RubberbandContext {
  public:
    const RubberbandContext* m_parent;
    RubberbandLayerContext* m_layerContext;
    const LayoutObject* m_layoutObject;
    const LayoutBlock* m_containingBlock;
    LayoutPoint m_layoutTopLeft;  // relative to the layer's top-left

    RubberbandContext()
    : m_parent(0)
    , m_layerContext(0)
    , m_layoutObject(0)
    , m_containingBlock(0)
    {
    }

    explicit RubberbandContext(const RubberbandContext* parent, const LayoutObject* layoutObject)
    : m_parent(parent)
    , m_layoutObject(layoutObject)
    , m_containingBlock(layoutObject ? layoutObject->containingBlock() : 0)
    {
        if (m_layoutObject && m_layoutObject->hasLayer()) {
            m_layerContext = new RubberbandLayerContext(parent->m_layerContext);
        }
        else {
            m_layerContext = parent->m_layerContext;
            m_layoutTopLeft = parent->m_layoutTopLeft;
        }
    }

    ~RubberbandContext()
    {
        if (m_layoutObject && m_layoutObject->hasLayer())
            delete m_layerContext;
    }

    LayoutPoint calcAbsPoint(const LayoutPoint& pt) const
    {
        LayoutUnit x = m_layerContext->m_translateX
                        - m_layerContext->m_frameScrollOffset.width()
                        - m_layerContext->m_layerScrollOffset.width()
                        + ((pt.x() + m_layoutTopLeft.x()) * m_layerContext->m_scaleX);
        LayoutUnit y = m_layerContext->m_translateY
                        - m_layerContext->m_frameScrollOffset.height()
                        - m_layerContext->m_layerScrollOffset.height()
                        + ((pt.y() + m_layoutTopLeft.y()) * m_layerContext->m_scaleY);
        LayoutPoint result(x, y);
        if (m_layerContext->m_colBlock) {
            LayoutPoint tmp = result;
            tmp.moveBy(-m_layerContext->m_colBlockAbsTopLeft);
            LayoutSize offset = m_layerContext->m_colBlock->columnOffset(tmp);
            result.move(offset);
        }
        return result;
    }

    LayoutRect calcAbsRect(const LayoutRect& rc) const
    {
        LayoutPoint topLeft = calcAbsPoint(rc.minXMinYCorner());
        LayoutSize size = rc.size();
        size.scale(m_layerContext->m_scaleX, m_layerContext->m_scaleY);
        return LayoutRect(topLeft, size);
    }
};

static bool isClipped(EOverflow overflow)
{
    return overflow == OAUTO
        || overflow == OHIDDEN
        || overflow == OSCROLL;
}

static bool isSupportedTransform(const TransformationMatrix& matrix)
{
    // We support transforms that have scale and translate only.
    return 0.0 == matrix.m21() && 0.0 == matrix.m31()
        && 0.0 == matrix.m12() && 0.0 == matrix.m32()
        && 0.0 == matrix.m13() && 0.0 == matrix.m23() && 1.0 == matrix.m33() && 0.0 == matrix.m43()
        && 0.0 == matrix.m14() && 0.0 == matrix.m24() && 0.0 == matrix.m34() && 1.0 == matrix.m44()
        // Additionally, we don't support flips or zero-scales.
        && 0.0 < matrix.m11() && 0.0 < matrix.m22();
}

static bool isTextRubberbandable(LayoutObject* layoutObject)
{
    return !layoutObject->style()
        || RUBBERBANDABLE_TEXT == layoutObject->style()->rubberbandable()
        || RUBBERBANDABLE_TEXT_WITH_LEADING_TAB == layoutObject->style()->rubberbandable();
}

static bool isTextWithLeadingTab(LayoutObject* layoutObject)
{
    return !layoutObject->style()
        || RUBBERBANDABLE_TEXT_WITH_LEADING_TAB == layoutObject->style()->rubberbandable();
}

template <typename CHAR_TYPE>
static void appendWithoutNewlines(WTF::StringBuilder *builder, const CHAR_TYPE* chars, unsigned length)
{
    builder->reserveCapacity(length);
    const CHAR_TYPE* end = chars + length;
    while (chars != end) {
        if ('\n' != *chars) {
            builder->append(*chars);
        }
        ++chars;
    }
}

static IntRect getRubberbandRect(const IntPoint& start, const IntPoint& extent)
{
    IntRect rc;
    rc.shiftXEdgeTo(std::min(start.x(), extent.x()));
    rc.shiftMaxXEdgeTo(std::max(start.x(), extent.x()));
    rc.shiftYEdgeTo(std::min(start.y(), extent.y()));
    rc.shiftMaxYEdgeTo(std::max(start.y(), extent.y()));
    return rc;
}

void WebViewImpl::rubberbandWalkFrame(const RubberbandContext& context, LocalFrame* frame, const LayoutPoint& clientTopLeft)
{
    frame->document()->updateLayout();

    FrameView* view = frame->view();

    LayoutObject* layoutObject = frame->contentLayoutObject();
    if (!layoutObject || !layoutObject->hasLayer())
        return;

    RubberbandContext localContext(&context, 0);
    localContext.m_containingBlock = 0;
    localContext.m_parent = 0;
    localContext.m_layoutTopLeft = LayoutPoint::zero();

    RubberbandLayerContext layerContext;
    localContext.m_layerContext = &layerContext;

    if (context.m_layerContext) {
        layerContext.m_frameScrollOffset = context.m_layerContext->m_frameScrollOffset;
        layerContext.m_layerScrollOffset = context.m_layerContext->m_layerScrollOffset;
        layerContext.m_translateX = context.m_layerContext->m_translateX + clientTopLeft.x();
        layerContext.m_translateY = context.m_layerContext->m_translateY + clientTopLeft.y();
        layerContext.m_scaleX = context.m_layerContext->m_scaleX;
        layerContext.m_scaleY = context.m_layerContext->m_scaleY;

        layerContext.m_clipRect = localContext.calcAbsRect(LayoutRect(LayoutPoint(), LayoutSize(view->size())));
        layerContext.m_clipRect.intersect(context.m_layerContext->m_clipRect);
    }
    else {
        layerContext.m_translateX = clientTopLeft.x();
        layerContext.m_translateY = clientTopLeft.y();
        layerContext.m_clipRect = localContext.calcAbsRect(LayoutRect(LayoutPoint(), LayoutSize(view->size())));
    }

    if (layerContext.m_clipRect.isEmpty()) {
        return;
    }

    layerContext.m_frameScrollOffset.expand(view->scrollOffset().width() * layerContext.m_scaleX,
                                            view->scrollOffset().height() * layerContext.m_scaleY);

    rubberbandWalkLayoutObject(localContext, layoutObject);
}

void WebViewImpl::rubberbandWalkLayoutObject(const RubberbandContext& context, LayoutObject* layoutObject)
{
    RubberbandContext localContext(&context, layoutObject);

    if (layoutObject->hasLayer()) {
        ASSERT(layoutObject->isLayoutBoxModelObject());
        DeprecatedPaintLayer* layer = toLayoutBoxModelObject(layoutObject)->layer();
        RubberbandLayerContext& layerContext = *localContext.m_layerContext;

        if (layer->hasTransformRelatedProperty()) {
            TransformationMatrix matrix = layer->currentTransform();
            if (!isSupportedTransform(matrix)) {
                return;
            }

            layerContext.m_translateX += layerContext.m_scaleX * (matrix.m41() + layer->location().x());
            layerContext.m_translateY += layerContext.m_scaleY * (matrix.m42() + layer->location().y());
            layerContext.m_scaleX *= matrix.m11();
            layerContext.m_scaleY *= matrix.m22();
        }
        else if (layer->location() != LayoutPoint::zero()) {
            layerContext.m_translateX += layerContext.m_scaleX * layer->location().x();
            layerContext.m_translateY += layerContext.m_scaleY * layer->location().y();
        }

        // TODO: how should we clip layers that are in columns?
        if (layer->layoutObject()->style() && !layerContext.m_colBlock) {
            bool isClippedX = isClipped(layer->layoutObject()->style()->overflowX());
            bool isClippedY = isClipped(layer->layoutObject()->style()->overflowY());
            if (isClippedX || isClippedY) {
                LayoutPoint minXminY = localContext.calcAbsPoint(LayoutPoint::zero());
                if (isClippedX) {
                    LayoutUnit minX = minXminY.x();
                    LayoutUnit maxX = minX + (layer->size().width() * layerContext.m_scaleX);
                    layerContext.m_clipRect.shiftXEdgeTo(std::max(minX, layerContext.m_clipRect.x()));
                    layerContext.m_clipRect.shiftMaxXEdgeTo(std::min(maxX, layerContext.m_clipRect.maxX()));
                }
                if (isClippedY) {
                    LayoutUnit minY = minXminY.y();
                    LayoutUnit maxY = minY + (layer->size().height() * layerContext.m_scaleY);
                    layerContext.m_clipRect.shiftYEdgeTo(std::max(minY, layerContext.m_clipRect.y()));
                    layerContext.m_clipRect.shiftMaxYEdgeTo(std::min(maxY, layerContext.m_clipRect.maxY()));
                }

                if (layerContext.m_clipRect.isEmpty()) {
                    return;
                }
            }
        }

        layerContext.m_layerScrollOffset.setWidth(layer->scrollableArea()->scrollXOffset() * layerContext.m_scaleX);
        layerContext.m_layerScrollOffset.setHeight(layer->scrollableArea()->scrollYOffset() * layerContext.m_scaleY);
    }
    else if (localContext.m_containingBlock != context.m_containingBlock) {
        ASSERT(localContext.m_containingBlock);
        const LayoutBlock* cb = localContext.m_containingBlock;
        const RubberbandContext* cbContext = &context;
        while (cbContext->m_layoutObject != cb) {
            ASSERT(cbContext->m_parent);
            cbContext = cbContext->m_parent;
        }

        ASSERT(cbContext->m_parent);
        ASSERT(cbContext->m_parent->m_layerContext);

        LayoutPoint offset;
        if (!cb->hasLayer() && cbContext->m_layerContext == localContext.m_layerContext)
            offset.move(cb->frameRect().x(), cb->frameRect().y());

        // undo previous containing block offsets (TODO)


        localContext.m_layoutTopLeft.moveBy(offset);
    }

    bool isVisible = !layoutObject->style() || layoutObject->style()->visibility() == VISIBLE;

    if (layoutObject->isBox()) {
        LayoutBox* layoutBox = toLayoutBox(layoutObject);

        // HACK: LayoutTableSection is not a containing block, but the cell
        //       positions seem to be relative to LayoutTableSection instead of
        //       LayoutTable.  This hack moves the top-left corner by the x,y
        //       position of the LayoutTableSection.
        if (layoutObject->isTableSection() && !layoutObject->hasLayer()) {
            localContext.m_layoutTopLeft.move(layoutBox->frameRect().x(), layoutBox->frameRect().y());
        }

        if (layoutObject->isLayoutIFrame() && isVisible) {
            LayoutIFrame* layoutIFrame = toLayoutIFrame(layoutObject);
            if (layoutIFrame->widget() && layoutIFrame->widget()->isFrameView()) {
                FrameView* frameView = static_cast<FrameView*>(layoutIFrame->widget());
                LayoutPoint topLeft;
                if (!layoutObject->hasLayer()) {
                    topLeft.move((localContext.m_layoutTopLeft.x() + layoutBox->frameRect().x()) * context.m_layerContext->m_scaleX,
                                 (localContext.m_layoutTopLeft.y() + layoutBox->frameRect().y()) * context.m_layerContext->m_scaleY);
                }
                topLeft.move((layoutBox->borderLeft() + layoutBox->paddingLeft()) * localContext.m_layerContext->m_scaleX,
                             (layoutBox->borderTop() + layoutBox->paddingTop()) * localContext.m_layerContext->m_scaleY);
                rubberbandWalkFrame(localContext, &frameView->frame(), topLeft);
            }
        }

        if (layoutObject->hasColumns() && layoutObject->isLayoutBlock()) {
            localContext.m_layerContext->m_colBlockAbsTopLeft = localContext.calcAbsPoint(LayoutPoint::zero());
            localContext.m_layerContext->m_colBlock = toLayoutBlock(layoutObject);
        }
    }
    else if (layoutObject->isText() && isVisible && isTextRubberbandable(layoutObject)) {
        LayoutText* layoutText = toLayoutText(layoutObject);

        WTF::String text(layoutText->text());
        for (InlineTextBox* textBox = layoutText->firstTextBox(); textBox; textBox = textBox->nextTextBox()) {
            LayoutUnit textBoxTop = textBox->root().lineTop();
            LayoutUnit textBoxHeight = textBox->root().lineBottom() - textBoxTop;
            LayoutUnit textBoxLeft = textBox->x();
            LayoutUnit textBoxWidth = textBox->width();

            LayoutRect localRect(textBoxLeft, textBoxTop, textBoxWidth, textBoxHeight);
            LayoutRect absRect = localContext.calcAbsRect(localRect);
            if (absRect.intersects(localContext.m_layerContext->m_clipRect)) {
                m_rubberbandState->m_impl->m_candidates.push_back(RubberbandCandidate());

                RubberbandCandidate& candidate = m_rubberbandState->m_impl->m_candidates.back();
                candidate.m_absRect = absRect;
                candidate.m_clipRect = candidate.m_absRect;
                candidate.m_clipRect.intersect(localContext.m_layerContext->m_clipRect);
                candidate.m_text = text;
                candidate.m_isLTR = textBox->isLeftToRightDirection();
                candidate.m_useLeadingTab = isTextWithLeadingTab(layoutText);
                candidate.m_start = textBox->start();
                candidate.m_len = textBox->len();
                int end = candidate.m_start + candidate.m_len;
                for (int offset = candidate.m_start; offset <= end; ++offset) {
                    LayoutUnit pos = textBox->positionForOffset(offset) - textBox->logicalLeft();
                    pos *= localContext.m_layerContext->m_scaleX;
                    if (candidate.m_isLTR)
                        pos += candidate.m_absRect.x();
                    else
                        pos = candidate.m_absRect.maxX() - pos;
                    candidate.m_charPositions.push_back(pos);
                }

                {
                    const Font& font = layoutObject->style()->font();
                    UChar space = ' ';
                    candidate.m_spaceWidth = font.width(
                        constructTextRun(layoutObject, font, &space, 1, layoutObject->styleRef(), candidate.m_isLTR ? LTR : RTL));
                    candidate.m_spaceWidth *= localContext.m_layerContext->m_scaleX;
                }
            }
        }
    }

    for (LayoutObject* child = layoutObject->slowFirstChild(); child; child = child->nextSibling()) {
        rubberbandWalkLayoutObject(localContext, child);
    }
}

WTF::String WebViewImpl::getTextInRubberbandImpl(const WebRect& rcOrig)
{
    ASSERT(isRubberbanding());

    LayoutRect rc(rcOrig);

    RubberbandStateImpl* stateImpl = m_rubberbandState->m_impl;

    std::vector<RubberbandCandidate> hits;
    for (std::size_t i = 0; i < stateImpl->m_candidates.size(); ++i) {
        const RubberbandCandidate& candidate = stateImpl->m_candidates[i];
        if (candidate.m_clipRect.intersects(rc)) {
            RubberbandCandidate hit = candidate;
            hit.m_clipRect.intersect(rc);
            hits.push_back(hit);
        }
    }

    if (hits.empty()) {
        return WTF::emptyString();
    }

    std::sort(hits.begin(), hits.end(), RubberbandCandidate_YSorter());
    LayoutUnit firstHitTop = hits[0].m_absRect.y();
    LayoutUnit firstHitHeight = hits[0].m_absRect.height();

    std::vector<std::size_t> lineBreaks;
    {
        LayoutUnit currLineTop = hits[0].m_absRect.y();
        LayoutUnit currLineBottom = hits[0].m_absRect.maxY();
        std::size_t currLineStart = 0;
        for (std::size_t i = 1; i < hits.size(); ++i) {
            const RubberbandCandidate& hit = hits[i];

            ASSERT(currLineTop <= hit.m_absRect.y());

            LayoutUnit maxTop = std::max(currLineTop, hit.m_absRect.y());
            LayoutUnit minBottom = std::min(currLineBottom, hit.m_absRect.maxY());
            LayoutUnit potentialLineHeight = std::max(currLineBottom, hit.m_absRect.maxY()) - currLineTop;
            if (minBottom - maxTop > std::min(hit.m_absRect.height(), potentialLineHeight) / 2) {
                // It overlaps sufficiently in the Y direction.  Now, if it
                // doesn't overlap with anything on the current line in the X
                // direction, we can keep this on the same row.
                bool overlapsInXDirection = false;
                if (!hit.isAllWhitespaces()) {
                    for (std::size_t j = currLineStart; j < i; ++j) {
                        const RubberbandCandidate& xtest = hits[j];
                        if (xtest.isAllWhitespaces()) {
                            continue;
                        }
                        LayoutUnit maxLeft = std::max(xtest.m_clipRect.x(), hit.m_clipRect.x());
                        LayoutUnit minRight = std::min(xtest.m_clipRect.maxX(), hit.m_clipRect.maxX());
                        if (minRight > maxLeft) {
                            overlapsInXDirection = true;
                            break;
                        }
                    }
                }
                if (!overlapsInXDirection) {
                    currLineBottom = std::max(currLineBottom, hit.m_absRect.maxY());
                    continue;
                }
            }

            lineBreaks.push_back(i);
            currLineTop = hit.m_absRect.y();
            currLineBottom = hit.m_absRect.maxY();
            currLineStart = i;
        }
    }

    {
        std::size_t lastLineBreak = 0;
        for (std::size_t i = 0; i < lineBreaks.size(); ++i) {
            std::sort(hits.begin() + lastLineBreak,
                      hits.begin() + lineBreaks[i],
                      RubberbandCandidate_XSorter());
            lastLineBreak = lineBreaks[i];
        }
        std::sort(hits.begin() + lastLineBreak, hits.end(), RubberbandCandidate_XSorter());
    }

    WTF::StringBuilder builder;
    LayoutUnit lastX = rc.x();
    bool lastHitWasAllWhitespaces = false;

    // Add sufficient newlines to get from the top of the rubberband to the first hit.
    {
        LayoutUnit y = rc.y() + firstHitHeight;
        while (y <= firstHitTop) {
            builder.append('\n');
            y += firstHitHeight;
        }
    }

    LayoutUnit lineTop = hits[0].m_absRect.y();
    LayoutUnit lineBottom = hits[0].m_absRect.maxY();

    std::size_t nextLineBreak = 0;
    for (std::size_t i = 0; i < hits.size(); ++i) {
        const RubberbandCandidate& hit = hits[i];

        if (nextLineBreak < lineBreaks.size() && i == lineBreaks[nextLineBreak]) {
            ++nextLineBreak;
            builder.append('\n');
            lastX = rc.x();

            LayoutUnit lastLineHeight = lineBottom - lineTop;
            LayoutUnit y = lineBottom + lastLineHeight;
            while (y < hit.m_absRect.y()) {
                builder.append('\n');
                y += lastLineHeight;
            }

            lineTop = hit.m_absRect.y();
            lineBottom = hit.m_absRect.maxY();
        }
        else {
            lineTop = std::min(lineTop, hit.m_absRect.y());
            lineBottom = std::max(lineBottom, hit.m_absRect.maxY());
        }

        ASSERT(lastX <= hit.m_clipRect.x() || hit.isAllWhitespaces() || lastHitWasAllWhitespaces);

        if (hit.m_useLeadingTab) {
            if (hit.m_clipRect.x() > rc.x()) {
                builder.append('\t');
            }
        }
        else if (hit.m_clipRect.x() > lastX) {
            LayoutUnit x = lastX + hit.m_spaceWidth;
            while (x <= hit.m_clipRect.x()) {
                builder.append(' ');
                x += hit.m_spaceWidth;
            }
        }

        ASSERT(!hit.m_clipRect.isEmpty());
        ASSERT(hit.m_clipRect.x() >= hit.m_absRect.x());
        ASSERT(hit.m_clipRect.maxX() <= hit.m_absRect.maxX());

        if (hit.m_clipRect.x() == hit.m_absRect.x() && hit.m_clipRect.maxX() == hit.m_absRect.maxX()) {
            if (hit.m_text.is8Bit()) {
                appendWithoutNewlines(&builder, hit.m_text.characters8() + hit.m_start, hit.m_len);
            }
            else {
                appendWithoutNewlines(&builder, hit.m_text.characters16() + hit.m_start, hit.m_len);
            }
        }
        else {
            int startOffset = 0, endOffset = hit.m_len;
            if (hit.m_isLTR) {
                for (std::size_t j = 0; j < hit.m_charPositions.size() - 1; ++j) {
                    LayoutUnit minX = hit.m_charPositions[j];
                    LayoutUnit maxX = hit.m_charPositions[j+1];
                    if (hit.m_clipRect.x() >= minX && hit.m_clipRect.x() < maxX) {
                        startOffset = j;
                    }
                    if (hit.m_clipRect.maxX() > minX && hit.m_clipRect.maxX() <= maxX) {
                        endOffset = j + 1;
                        break;
                    }
                }
            }
            else {
                for (std::size_t j = 0; j < hit.m_charPositions.size() - 1; ++j) {
                    LayoutUnit maxX = hit.m_charPositions[j];
                    LayoutUnit minX = hit.m_charPositions[j+1];
                    if (hit.m_clipRect.maxX() > minX && hit.m_clipRect.maxX() <= maxX) {
                        startOffset = j + 1;
                    }
                    if (hit.m_clipRect.x() >= minX && hit.m_clipRect.x() < maxX) {
                        endOffset = j;
                        break;
                    }
                }
            }

            ASSERT(startOffset <= endOffset);
            ASSERT(0 <= startOffset);
            ASSERT(startOffset <= hit.m_len);
            ASSERT(0 <= endOffset);
            ASSERT(endOffset <= hit.m_len);

            if (hit.m_text.is8Bit()) {
                appendWithoutNewlines(&builder,
                                      hit.m_text.characters8() + hit.m_start + startOffset,
                                      endOffset - startOffset);
            }
            else {
                appendWithoutNewlines(&builder,
                                      hit.m_text.characters16() + hit.m_start + startOffset,
                                      endOffset - startOffset);
            }
        }

        lastX = hit.m_clipRect.maxX();
        lastHitWasAllWhitespaces = hit.isAllWhitespaces();
    }

    return builder.toString();
}

bool WebViewImpl::handleAltDragRubberbandEvent(const WebInputEvent& inputEvent)
{
    if (!(inputEvent.modifiers & WebInputEvent::AltKey)) {
        if (isRubberbanding()) {
            if (m_client)
                m_client->hideRubberbandRect();
            abortRubberbanding();
        }
        return false;
    }

    if (!WebInputEvent::isMouseEventType(inputEvent.type))
        return false;

    const WebMouseEvent& mouseEvent = *static_cast<const WebMouseEvent*>(&inputEvent);

    if (!isRubberbanding()) {
        if (inputEvent.type == WebInputEvent::MouseDown && preStartRubberbanding()) {
            startRubberbanding();
            m_rubberbandState->m_impl->m_startPoint = IntPoint(mouseEvent.x, mouseEvent.y);
            return true;
        }

        return false;
    }
    else if (inputEvent.type == WebInputEvent::MouseUp) {
        if (m_client)
            m_client->hideRubberbandRect();

        IntPoint start = m_rubberbandState->m_impl->m_startPoint;
        IntPoint extent = IntPoint(mouseEvent.x, mouseEvent.y);
        WebRect rc = expandRubberbandRect(getRubberbandRect(start, extent));
        if (rc.isEmpty()) {
            abortRubberbanding();
        }
        else {
            WebString copied = finishRubberbanding(rc);
            Pasteboard::generalPasteboard()->writePlainText(copied, Pasteboard::CannotSmartReplace);
        }

        return true;
    }
    else {
        if (m_client) {
            IntPoint start = m_rubberbandState->m_impl->m_startPoint;
            IntPoint extent = IntPoint(mouseEvent.x, mouseEvent.y);
            WebRect rc = expandRubberbandRect(getRubberbandRect(start, extent));
            m_client->setRubberbandRect(rc);
        }
        return true;
    }
}

bool WebViewImpl::isAltDragRubberbandingEnabled() const
{
    return m_isAltDragRubberbandingEnabled;
}

void WebViewImpl::enableAltDragRubberbanding(bool value)
{
    m_isAltDragRubberbandingEnabled = value;
    if (!m_isAltDragRubberbandingEnabled && isRubberbanding()) {
        if (m_client)
            m_client->hideRubberbandRect();
        abortRubberbanding();
    }
}

bool WebViewImpl::isRubberbanding() const
{
    return m_rubberbandState.get();
}

bool WebViewImpl::preStartRubberbanding()
{
    ASSERT(!isRubberbanding());

    if (!m_page || !m_page->mainFrame() || !m_page->mainFrame()->isLocalFrame() || !m_page->deprecatedLocalMainFrame()->document())
        return false;

    RefPtr<Event> event = Event::createCancelable("rubberbandstarting");
    if (!m_page->deprecatedLocalMainFrame()->document()->dispatchEvent(event))
        return false;

    return !event->defaultPrevented();
}

void WebViewImpl::startRubberbanding()
{
    ASSERT(!isRubberbanding());

    m_rubberbandState = adoptPtr(new RubberbandState());

    RubberbandContext context;
    rubberbandWalkFrame(context, m_page->deprecatedLocalMainFrame(), LayoutPoint());
}

WebRect WebViewImpl::expandRubberbandRect(const WebRect& rcOrig)
{
    ASSERT(isRubberbanding());

    LayoutRect rc(rcOrig);

    RubberbandStateImpl* stateImpl = m_rubberbandState->m_impl;

    std::size_t i = 0;
    while (i < stateImpl->m_candidates.size()) {
        const RubberbandCandidate& candidate = stateImpl->m_candidates[i];
        if (candidate.m_clipRect.intersects(rc)) {
            bool expanded = false;
            LayoutUnit y = candidate.m_absRect.y();
            LayoutUnit maxY = candidate.m_absRect.maxY();
            if (rc.y() > y) {
                rc.shiftYEdgeTo(y);
                expanded = true;
            }
            if (rc.maxY() < maxY) {
                rc.shiftMaxYEdgeTo(maxY);
                expanded = true;
            }
            if (expanded) {
                // since we expanded the rect, there might be more candidates that get included
                i = 0;
                continue;
            }
        }
        ++i;
    }

    LayoutUnit minX = rc.x();
    LayoutUnit maxX = rc.maxX();

    for (i = 0; i < stateImpl->m_candidates.size(); ++i) {
        RubberbandCandidate& candidate = stateImpl->m_candidates[i];
        if (candidate.m_clipRect.intersects(rc)) {
            for (std::size_t j = 0; j < candidate.m_charPositions.size() - 1; ++j) {
                LayoutUnit startOfChar = candidate.m_charPositions[j];
                LayoutUnit endOfChar = candidate.m_charPositions[j+1];
                if (candidate.m_isLTR) {
                    if (rc.x() > startOfChar && rc.x() < endOfChar) {
                        minX = std::min(minX, startOfChar + 1);
                    }
                    if (rc.maxX() > startOfChar && rc.maxX() < endOfChar) {
                        maxX = std::max(maxX, endOfChar);
                    }
                }
                else {
                    if (rc.x() > endOfChar && rc.x() < startOfChar) {
                        minX = std::min(minX, endOfChar);
                    }
                    if (rc.maxX() > endOfChar && rc.maxX() < startOfChar) {
                        maxX = std::max(maxX, startOfChar - 1);
                    }
                }
            }
        }
    }

    rc.shiftXEdgeTo(minX);
    rc.shiftMaxXEdgeTo(maxX);

    return pixelSnappedIntRect(rc);
}

WebString WebViewImpl::finishRubberbanding(const WebRect& rc)
{
    ASSERT(isRubberbanding());

    WTF::String copied = getTextInRubberbandImpl(rc);

    m_rubberbandState.clear();
    if (m_page && m_page->mainFrame() && m_page->mainFrame()->isLocalFrame() && m_page->deprecatedLocalMainFrame()->document()) {
        RefPtr<Event> event = Event::create("rubberbandfinished");
        m_page->deprecatedLocalMainFrame()->document()->dispatchEvent(event);
    }

    return copied;
}

void WebViewImpl::abortRubberbanding()
{
    ASSERT(isRubberbanding());

    m_rubberbandState.clear();

    if (m_page && m_page->mainFrame() && m_page->mainFrame()->isLocalFrame() && m_page->deprecatedLocalMainFrame()->document()) {
        RefPtr<Event> event = Event::create("rubberbandaborted");
        m_page->deprecatedLocalMainFrame()->document()->dispatchEvent(event);
    }
}

WebString WebViewImpl::getTextInRubberband(const WebRect& rc)
{
    ASSERT(!isRubberbanding());

    if (!m_page || !m_page->mainFrame() || rc.isEmpty() || !m_page->mainFrame()->isLocalFrame())
        return WTF::emptyString();

    m_rubberbandState = adoptPtr(new RubberbandState());

    RubberbandContext context;
    RubberbandLayerContext layerContext;
    context.m_layerContext = &layerContext;
    layerContext.m_clipRect = LayoutRect(rc);
    rubberbandWalkFrame(context, m_page->deprecatedLocalMainFrame(), LayoutPoint());
    WTF::String result = getTextInRubberbandImpl(rc);
    m_rubberbandState.clear();
    return result;
}

} // namespace blink
