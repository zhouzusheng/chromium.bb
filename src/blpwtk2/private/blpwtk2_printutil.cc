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

#include <blpwtk2_printutil.h>
#include <third_party/skia/include/core/SkPaint.h>
#include <third_party/skia/include/core/SkCanvas.h>
#include <third_party/skia/include/core/SkTypeface.h>
#include <third_party/skia/include/core/SkTypeface.h>
#include <skia/ext/platform_canvas.h>
#include <third_party/WebKit/Source/Platform/chromium/public/WebCanvas.h>
#include <third_party/WebKit/Source/Platform/chromium/public/WebSize.h>
#include <third_party/WebKit/Source/Platform/chromium/public/WebString.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebBBPrintInfo.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebPrintParams.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebView.h>
#include <content/public/renderer/render_view.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <windows.h>

namespace blpwtk2 {

static void printMarginBoxPart(WebKit::WebCanvas *canvas,
                               SkPaint &skPaint,
                               int pageToPrint,
                               int numPages,
                               const WebKit::WebString &formatText,
                               WebKit::WebBBPrintHeader::Align align,
                               double pageMarginLeft,
                               double pageMarginRight,
                               double yPos)
{
    // Construct paging string, e.g "Page 1 of 4"
    WCHAR buf[1024];

    if (formatText.isEmpty())
        return;

    wnsprintf(buf, sizeof(buf) / sizeof(buf[0]), formatText.data(), pageToPrint + 1, numPages);

    std::wstring text(buf);

    if (text.empty())
        return;

    const double headerTextWidth = skPaint.measureText(text.c_str(), 2 * text.size());
    double headerX = 0;

    switch (align) {
        case WebKit::WebBBPrintHeader::LEFT:
            headerX = pageMarginLeft;
            break;
        case WebKit::WebBBPrintHeader::CENTER:
            headerX = (pageMarginLeft + pageMarginRight - headerTextWidth) / 2;
            break;
        case WebKit::WebBBPrintHeader::RIGHT:
            headerX = pageMarginRight - headerTextWidth;
            break;
    }

    canvas->drawText(text.c_str(), 2 * text.size(), headerX, yPos, skPaint);
}

static void printMarginBox(WebKit::WebCanvas *canvas,
                           int pageToPrint,
                           int numPages,
                           double pageMarginLeft,
                           double pageMarginRight,
                           const WebKit::WebBBPrintHeader& data,
                           double yPos)
{
    // Draw string
    SkAutoTUnref<SkTypeface> skTypeFace(SkTypeface::CreateFromName(data.fontFamily().utf8().data(), SkTypeface::kNormal));

    SkPaint skPaint;
    skPaint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    skPaint.setTypeface(skTypeFace);
    skPaint.setTextSize(data.fontSize());
    skPaint.setColor(data.color());

    printMarginBoxPart(canvas,
                       skPaint,
                       pageToPrint,
                       numPages,
                       data.text(),
                       (WebKit::WebBBPrintHeader::Align)data.align(),
                       pageMarginLeft,
                       pageMarginRight,
                       yPos);
}

static HDC getPrintDC()
{
    // from http://msdn.microsoft.com/en-us/library/windows/desktop/dd162833(v=vs.85).aspx
    PRINTDLG printDialog;
    ::ZeroMemory(&printDialog, sizeof(printDialog));
    printDialog.lStructSize = sizeof(printDialog);
    printDialog.nCopies = 1;
    printDialog.Flags =
        // Return a printer device context
        PD_RETURNDC
        // Don't allow separate print to file.
        | PD_HIDEPRINTTOFILE
        | PD_DISABLEPRINTTOFILE
        // Don't allow selecting individual document pages to print.
        | PD_NOSELECTION;

    BOOL shouldPrint = ::PrintDlg(&printDialog);
    if (!shouldPrint) {
        return NULL;
    }
    return printDialog.hDC;
}

static double printToScreen(double printPixels, double printPPI)
{
    const double screenPPI = 96.0;

    return printPixels * screenPPI / printPPI;
}

static void printHeader(WebKit::WebCanvas *canvas,
                        int pageToPrint,
                        int numPages,
                        double pageMarginLeft,
                        double pageMarginRight,
                        const WebKit::WebBBPrintHeader& data)
{
    const double headerY = (data.verticalMargin() + data.fontSize());
    printMarginBox(canvas, pageToPrint, numPages, pageMarginLeft, pageMarginRight, data, headerY);
}

static void printFooter(WebKit::WebCanvas *canvas,
                        int pageToPrint,
                        int numPages,
                        double pageMarginLeft,
                        double pageMarginRight,
                        const WebKit::WebBBPrintHeader& data,
                        const WebKit::WebSize& displaySize)
{
    const double footerY = displaySize.height - data.verticalMargin();
    printMarginBox(canvas, pageToPrint, numPages, pageMarginLeft, pageMarginRight, data, footerY);
}

void PrintUtil::PrintPage(WebKit::WebFrame* frame, WebKit::WebView* view)
{
    WebKit::WebBBPrintInfo bbPrintInfo;

    if (view) {
        bbPrintInfo = view->mainFrame()->document().bbPrintInfo();
    }

    HDC printDc = getPrintDC();

    if (printDc == NULL) {
        return;
    }

    const int w = GetDeviceCaps(printDc, HORZRES);
    const int h = GetDeviceCaps(printDc, VERTRES);
    const int dpi = GetDeviceCaps(printDc, LOGPIXELSX);

    const double pointsPerInch = 96.0;
    const double dotsPerPoint = dpi / pointsPerInch;
    const double displayScaleFactor = dotsPerPoint;


    WebKit::WebSize displaySize(printToScreen(w, dpi), printToScreen(h, dpi));
    int mt = 0, mb = 0, ml = 0, mr = 0;
    frame->pageSizeAndMarginsInPixels(
        0,
        displaySize,
        mt, mr, mb, ml);


    const double printWidthPoints = displaySize.width - ml - mr;
    const double printHeightPoints = displaySize.height - mt - mb;
    WebKit::WebSize size(printWidthPoints, printHeightPoints);

    WebKit::WebPrintParams params(size);
    params.printerDPI = dpi;

    WebKit::WebNode node;
    bool useBrowserOverlays;
    int pageCount = frame->printBegin(params, node, &useBrowserOverlays);

    // TODO: Use printDc to draw directly into?
    WebKit::WebCanvas *canvas = skia::CreateBitmapCanvas(w, h, true);

    DOCINFO docInfo = {0};
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = frame->document().title().data();

    if (::StartDoc(printDc, &docInfo) > 0) {

        for (int pageToPrint = 0; pageToPrint < pageCount; ++pageToPrint) {

            if (::StartPage(printDc) > 0) {

                // Blank the margins
                {
                    SkRect r = SkRect::MakeWH(w, h);
                    SkPaint p;
                    p.setColor(SK_ColorWHITE);

                    canvas->drawRect(r, p);
                }

                // Draw the header & footer
                {
                    SkAutoCanvasRestore canvasStore(canvas, true);

                    canvas->scale(displayScaleFactor, displayScaleFactor);

                    printHeader(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.headerLeft());
                    printHeader(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.headerCenter());
                    printHeader(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.headerRight());
                    printFooter(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.footerLeft(), displaySize);
                    printFooter(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.footerCenter(), displaySize);
                    printFooter(canvas, pageToPrint, pageCount, ml, mr, bbPrintInfo.footerRight(), displaySize);
                }

                // Draw the page
                {
                    SkAutoCanvasRestore canvasStore(canvas, true);

                    const double shrinkFactor = frame->getPrintPageShrink(pageToPrint);
                    if (0.0f == shrinkFactor) { // zero if page number invalid or not printing
                        break;
                    }

                    canvas->scale(displayScaleFactor, displayScaleFactor);
                    canvas->translate(ml, mt);
                    canvas->scale(shrinkFactor, shrinkFactor);

                    frame->printPage(pageToPrint, canvas);

                    skia::DrawToNativeContext(canvas, printDc, 0, 0, NULL);

                    if (::EndPage(printDc) == 0) {
                        break;
                    }
                }
            }
        }

        ::EndDoc(printDc);
    }

    ::DeleteDC(printDc);

    frame->printEnd();
}

}  // close namespace blpwtk2
