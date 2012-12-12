# chromium.bb

This repository contains the version of Chromium used by Bloomberg, along with
all the modifications made by Bloomberg in order to use it in our environment.

This repository serves a couple of purposes for us:

* **Provide a trimmed snapshot of the Chromium tree**
  A typical Chromium checkout is about 3GB and fetches code from several
different repositories.  Most of that code is not used by Bloomberg (we only
use the `test_shell` portion of Chromium).

  Our checkout is about 260MB, and this all comes from a single repo, which
makes it much easier for us to use internally.

* **Provide a space for us to make/publish changes**
  We have made a bunch of changes to different parts of Chromium in order to
make it behave the way we need it to.  Some of these changes are bugfixes that
can be submitted upstream, and some of them are just changes that we
specifically wanted for our product, but may not be useful or desirable
upstream.  Each change is made on a separate branch.  This allows us, at
release time, to pick and choose which bugfixes/features we want to include in
the release.

  Note that while most of our bugfixes are not submitted upstream, it is our
intention to submit as many bugfixes upstream as we can.


## Overall Structure

The structure of this repository is somewhat unconventional, but it serves our
purpose well.  The `master` branch is not really used (it is actually used by
an internal buildbot, but is otherwise not very useful).

Our real entry points are the snapshots we get from upstream.  We tag each of
these snapshots using the format `upstream/<channel>/<version>`, for example:
`upstream/stable/21.0.1180.60`.

Each tag we snapshot is committed on top of the tag for the previous version.
This essentially forms a linear branch of upstream versions.

All our `bugfix/` and `feature/` branches are based off one of these tags.  The
version number of the base tag is appended to the name of the branch, for
example: `bugfix/offsetleft/19.0.1084.52` is based off the tag
`upstream/stable/19.0.1084.52`.

When we make a release, we first decide which upstream tag to base the release
from.  Our merge tool then selects the branches corresponding to that version,
and also includes the branches from any previous versions.  For example, when
making a release based on `19.0.1084.52`, we also include branches that were
based on `18.0.1025.162`, but not branches that were based on `20.0.1132.47`.

Sometimes, conflicts arise when we merge branches that were based on older
versions, due to other changes from upstream.  For example,
`bugfix/offsetleft/19.0.1084.52` does not apply cleanly over
`upstream/stable/20.0.1132.47`.  To resolve this, a new branch is created
called `bugfix/offsetleft/20.0.1132.47`, based off
`upstream/stable/20.0.1132.47`, and the previous branch
`bugfix/offsetleft/19.0.1084.52` is merged and the conflicts are resolved in
the new branch.  This way, the merge tool we use during releases can work
autonomously to pickup our `offsetleft` changes.


## Build Instructions

* Checkout one of the upstream tags (you should pick the latest version).
* Open `chromium/src/webkit/webkit.sln`.  This is the generated solution file
  for `test_shell` after running `gclient sync` on the original Chromium
  checkout.
* Build the `test_shell` project
* Now you can start merging branches that you're interested in (see branch
  descriptions below).
* Each branch introduces a specific fix, which you can test by running
  `test_shell` against the URL provided in the branch description.


## Branches

This section will describe each branch and what changes they introduce.  Note
that some of these changes have been contributed upstream.  Our intention is to
contribute as many changes upstream as possible so that we do not have to
maintain these branches in our repository.

However, we realize that some of these changes are made in order to adjust the
behavior of our own product, and may not be desirable/applicable to the general
web.  For example, the way the `<delete>`/`<backspace>` keys work inside table
cells, the way indenting/outdenting of list items work, etc.  Therefore, we
will probably not be able to send *everything* upstream.


### bugfix/autoCorrectionRange (Edward Christie)
TODO

### bugfix/autocorrectcaret2 (Calum Robinson)
TODO


### bugfix/backspaceIntoTable (Shezan Baig; D32947240)

This bug can be reproduced in http://jsfiddle.net/t2wNK/

Open this link in Chrome, and put the cursor between 'T' and 'e' in "Testing",
and hit backspace twice.  The rest of the text will go into the last cell.

It is unclear whether this is a bug that can be upstreamed or not, because the
comment in the code specifically mentions that this is the intended behavior,
and has been since at least 2007.  However, we changed this behavior in our
product to match what Firefox does, which is to make the second backspace a
no-op.


### bugfix/badquerycommand (Shezan Baig; D32128551)

This bug can be reproduced in http://jsfiddle.net/FRu9k/

Open this link in Chrome, click the yellow `div` and hit `<enter>` a bunch of
times.  Then `<shift>`+`<up>` a bunch of times.  Each time you press `<up>`,
the result of `queryCommandValue('bold')` changes inconsistently.


### bugfix/cancelErrorNotNull (Zhen Yin)
TODO


### bugfix/cellInMulticol (Shezan Baig; D34079255)

This bug can be reproduced in TODO

Open this link in Chrome and keep resizing the window.  You will see that the
table cells get split when they reach column boundaries.

This is a rather complicated patch and requires a bunch of additional work
before it can be sent upstream.


### bugfix/cleanWarnings (Shezan Baig)

The purpose of this branch is simply to cleanup the ton of compiler warnings we
get when building Chromium in Visual Studio.  Most of these warnings are
benign, so we simply just disable them.


### bugfix/clearDragCaret (Shezan Baig; D31916022)

This bug can be reproduced in http://jsfiddle.net/dQtQv/

Open this link in Chrome, then select some text from another tab, and try to
drag it into the editable div.  Before you drop, you will see a caret at the
place where the text should go.  When you drop however,
`event.preventDefault()` is called, so the text does not get inserted.
However, the drag caret remains visible.


### bugfix/cleartypecanvas (Shezan Baig; D32281407; upstream: [86776](https://bugs.webkit.org/show_bug.cgi?id=86776))

This bug can be reproduced in TODO (`canvas_text.html`)

Open this link in Chrome, and notice that the canvas element does not use
ClearType fonts even though `subpixel-antialiased` is used.

Note that when painting to an `ImageBuffer`, Chromium disables ClearType even
if it is the default on the system.  However, in our case, we are explicitly
requesting it in the document, so this change makes Chromium honor the
`-webkit-font-smoothing` setting if it is requested explicitly.

Note that there are two distinct changes introduced by this branch:

* `-webkit-font-smoothing` has no effect in Windows for Skia.
  This is the change that is being tracked upstream by WebKit bug
  [86776](https://bugs.webkit.org/show_bug.cgi?id=86776).

* Don't disable ClearType on `ImageBuffer` if it was explicitly requested
  This is not yet being tracked upstream.


### bugfix/contractionFix (Edward Christie)
TODO


### bugfix/cursorContext (Shezan Baig; D35952365)

This bug can be reproduced in TODO

It is somewhat hard to reproduce, the window height needs to be just about
right.

Open this link in Chrome, and hit Enter until the cursor is at the last
possible line where the scrollbar does not appear.  Make sure there is just
enough space for about half a line more.

Now keep typing on this line, letting it word-wrap which causes the scrollbar
to appear.  Keep doing this for a few lines.  You will notice that on some
lines (usually the second or the third, then every alternating line after
that), the scrollbar does not scroll down completely.

In our product, there is an overlay around the RTE of about a couple of pixels.
The purpose of the padding on the contenteditable was to prevent this overlay
from covering the text.  However, since the bottom of the text was right
against the border of the `content-editable`, this was causing the descent of
some characters to be obscured.

We "fixed" this in this branch by inflating the reveal rect by about half the
cursor height, showing more "context" when moving up and down in a scrollable
contenteditable.  However, a proper fix would be to make the last line scroll
completely, taking the padding into account.  We haven't gotten around to this
yet.


### bugfix/dontRespondIfNoFocus (Shezan Baig; D32819389)
TODO


### bugfix/doubleindent (Shezan Baig; D32165963)

This bug can be reproduced in TODO

Open this link in Chrome, and position the cursor before the logo.  Indent the
image twice, then outdent the image twice.  There will be one level of
indentation still left.


### bugfix/emptycellcaret (Shezan Baig; upstream: [85385](https://bugs.webkit.org/show_bug.cgi?id=85385))

This bug can be reproduced in http://jsfiddle.net/wceLA/

Open this link in Chrome, and put the caret inside an empty table cell.  The
caret's `y-position` is outside the `table-cell`.


### bugfix/fix-modal-loop (Imran Haider)
TODO


### bugfix/fixFlexRounding (Shezan Baig; upstream: [92163](https://bugs.webkit.org/show_bug.cgi?id=92163))

The purpose of this branch is to pull an upstream bugfix into our version of
Chromium, which lags behind the latest upstream version by quite a bit.


### bugfix/flexReplaced (Shezan Baig; D34372405; upstream: [94237](https://bugs.webkit.org/show_bug.cgi?id=94237))

This branch fixes an issue where replaced elements were not stretching properly
inside flexboxes.  It has been submitted upstream, so it cannot be reproduced
in Chrome anymore.


### bugfix/iframeZoom (Shezan Baig; D34484409)

This bug can be reproduced in TODO

Open the link in Chrome, and zoom the page in and out.  The content outside the
`iframe` zooms, but the content inside the `iframe` does not zoom.


### bugfix/imgresample (Shezan Baig; D32353761)

This bug can be reproduced in TODO

Open the link in Chrome.  The IMG element does not scale properly.

The issue is that the logic that determines the image resampling mode (no
resampling vs linear resampling) doesn't take into account the transformed
rectangle when a CSS scale has been applied.

Note: When testing this branch with an SVG gile, you may also need the changes
in the `bugfix/svgscale` branch in order for the fix to work correctly.


### bugfix/indentpre (Shezan Baig; D31922188)

This bug can be reproduced in TODO

Open the link in Chrome.  The "Hello" text will be selected on load.  Each time
you indent the text, the "Hello" will get duplicated.


### bugfix/justifyprewrap (Shezan Baig; D31916947; upstream: [84448](https://bugs.webkit.org/show_bug.cgi?id=84448))

This bug can be reproduced in TODO

Open the link in Chrome.  The second paragraph will not be justified, even
though `text-align: justify` is specified.

There is an upstream ticket for this issue, but it calls for a much more
comprehensive fix than what was done in this branch.  In this branch, we just
enable the existing justification logic in elements that have
`whitespace:pre-wrap` specified, but the upstream ticket calls for modification
to the justification logic itself to match how Microsoft Word software works.


### bugfix/listMarkerAlignment (Shezan Baig; D37088493; upstream: [21709](https://bugs.webkit.org/show_bug.cgi?id=21709))

This bug can be reproduced in TODO

Open the link in Chrome.  The list markers are all left-aligned, even though
the second and third `li` elements are not.


### bugfix/listMarkerFont (Shezan Baig; D32534047)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch makes it so that the font used for the list marker will be the same
as the first character in the list item.


### bugfix/listMarkerSelection (Shezan Baig; D32422051)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch makes it so that list markers are not highlighted when the list
item is selected.  This matches the behavior in Firefox and Microsoft Word
software.


### bugfix/mouseLeaveMousePosition (Zhen Yin)
TODO


### bugfix/noBRAtStartOfPara (Shezan Baig; D36813987)

This bug can be reproduced in TODO

Open the link in Chrome.  The contents of the table will be selected on load.
Hit the "List" button multiple times.  Each click executes the
`insertOrderedList` command.

On every second click, the table will move down.


### bugfix/nonWindowContexts (Shezan Baig)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch removes the assumption that every V8 context has a `DOMWindow`
associated with it.  This is necessary for our product because we have multiple
V8 contexts living in the same process that need to share data with each other.


### bugfix/nonemptycellbackspace (Shezan Baig)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch makes it so that hitting `<backspace>` when the cursor is at the
beginning of a table cell will be a no-op.  The default behavior in WebKit is
to move into the previous cell and start deleting from there.


### bugfix/nonemptycelldelete (Shezan Baig)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch makes it so that hitting `<delete>` when the cursor is at the end
of a table cell will be a no-op.  The default behavior in WebKit is to move
into the next cell and start deleting from there.


### bugfix/noRelayoutIfPercentSame (Shezan Baig; D36633185)
TODO


### bugfix/outdentWithBR (Shezan Baig; upstream: [92130](https://bugs.webkit.org/show_bug.cgi?id=92130))

This bug can be reproduced in https://bug-92130-attachments.webkit.org/attachment.cgi?id=154080

Open the link in Chrome.  The block will be outdented on load.  However, there
are now a bunch of empty lines added in between.


### bugfix/pasteUsesLineBreak (Shezan Baig; D33830147)
TODO

### bugfix/perl\_filenames (Edward Christie)
TODO

### bugfix/printBkTransparent (Shezan Baig; D32606365)
TODO

### bugfix/printTextTop (Shezan Baig; D32606285)
TODO

### bugfix/removeMarkersUponEditing (Edward Christie)
TODO


### bugfix/rowOutlines (Shezan Baig; D34115441; upstream: [94007](https://bugs.webkit.org/show_bug.cgi?id=94007))

The purpose of this branch is to pull an upstream bugfix into our version of
Chromium, which lags behind the latest upstream version by quite a bit.


### bugfix/scrollClipRect (Shezan Baig; D32861389)

This bug can be reproduced in TODO

It is somewhat hard to reproduce, the window size needs to be just about right.

Open the link in Chrome, and resize the window so that new outer scrollbars
appear over the existing scrollbars of the iframe.  Then resize it just a
little smaller.  Then use the mouse wheel to scroll the contents of the iframe.

If the window is sized just right, you will see that only a portion of the
iframe scrolls.  This is because the clipRect is not being calculated correctly
due to the scale transform being applied.


### bugfix/selectionGapTransform (Shezan Baig)
TODO


### bugfix/selrectantialias (Shezan Baig; D32113976; upstream: [87157](https://bugs.webkit.org/show_bug.cgi?id=87157))

This bug can be reproduced in TODO

Open the link in Chrome.  You will see "gaps" between the lines and the spans
on the first line.

The issue is that the selection highlight is drawn for each text run with
anti-aliasing on.  This causes "gaps" to appear around the edges where the
filled rects got blended.  The solution was to simply turn off anti-aliasing
when filling selection rects.


### bugfix/spellCheckOnLoad (Shezan Baig; D32468818)

Note: This branch has been miscategorized as a bugfix, it is actually a feature
that we wanted specifically for our product, so it is unlikely that we will
send this upstream.

This branch makes it so that spell-checking is performed when the document is
loaded.  WebKit's original behavior was to perform spell-check when the user
makes a change to the document.


### bugfix/svgscale (Shezan Baig; D32353761; upstream: [85335](https://bugs.webkit.org/show_bug.cgi?id=85335))

This bug can be reproduced in TODO

Open the link in Chrome.  When a SVG file is used as a css background, or in
an `img` element, it does not scale properly.  It only scales properly when
used in an `embed` element.


### bugfix/tablecellpaste (Shezan Baig; upstream: [53933](https://bugs.webkit.org/show_bug.cgi?id=53933))

This bug has been fixed upstream, so is no longer reproducable in Chrome.  The
issue was that pasting text into a table cell would paste the text into the
next cell instead.

TODO: The upstream fix was actually made in WebKit bug
[75004](https://bugs.webkit.org/show_bug.cgi?id=75004).  We should be using
that fix instead of our own.


### bugfix/tooltip\_refresh (Shezan Baig; upstream: [84375](https://bugs.webkit.org/show_bug.cgi?id=84375))

This bug can be reproduced in http://jsbin.com/omahe3

Open the link in Chrome.  Each line has a tooltip set.  However, when the first
tooltip shows up, and you move the mouse away from it, it doesn't move the
tooltip to the element where the mouse is.


### bugfix/unifiedTextChecking (Edward Christie)
TODO

### feature/after\_copycut (James McIlhargey)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This branch fires two new events `aftercut` and `aftercopy` which have
read/write access to the clipboard representation.  This allows us to intercept
the state of the clipboard after WebKit has populated it and make any tweaks.
This is useful for selectively overriding some copy/paste behaviour with
minimal JS code.

### feature/allowImageLoadCancel (Alex Buch; D32427286)
TODO

### feature/can\_supply\_proxy\_service (Angelos Evripiotis)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

We needed extra control over the proxy server configuration that's used by
the `SimpleResourceLoaderBridge`.  This branch makes it possible to supply a
custom `net::ProxyConfigService` so we can provide our own settings

### feature/colorDocumentMarkers (Shezan Baig; D32415776)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

We needed extra control of the color of the document markers (i.e. the squiggly
lines for spelling errors etc).  WebKit hardcodes spelling errors to red,
grammar errors to green.  This branch makes it possible for us to specify the
color from the document.

TODO: make this a CSS property instead of a DOM attribute.


### feature/disableV8SetAccessCheckCallbacks (Tianyin Zhang)
TODO

### feature/expose-request-context (Imran Haider)
TODO

### feature/fontSmoothing (Calum Robinson; D36278764)
TODO


### feature/indentOutdentBB (Shezan Baig; D36805707, D32177325)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This branch adds `IndentBB` and `OutdentBB` commands that make indenting and
outdenting of list items behave closer to how it works in MS Word.

TODO: The code for this is very messy, it needs to be cleaned up.


### feature/insertHTMLNested (Shezan Baig; D37136356)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

We needed to modify the behavior of the `InsertHTML` command.  The WebKit
behavior is to prevent the inserted HTML from being nested inside style spans.
This branch adds `InsertHTMLNested` command that allows the HTML to be nested
inside style spans.


### feature/instrumentLayout (Inseob Lee; D35009812)
TODO

### feature/instrumentRefCountedLeakCounter (Imran Haider)
TODO

### feature/line-erase (Imran Haider)
TODO


### feature/nColumnSpan (Shezan Baig; D33399541, D36821222)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

When using the multicolumn feature, there is a way to make an element span
across all columns (`-webkit-column-span: all`).

However, we needed a way to span across a specified number of columns, (e.g.
`-webkit-column-span: 2`).  Once CSS Regions have matured (WebKit bug
[57312](https://bugs.webkit.org/show_bug.cgi?id=57312)), we should be able to
use Regions to achieve this effect, then we can get rid of this branch.
However, we needed this one specific feature immediately, so this branch just
hacks it in.


### feature/noV8RecursionScopeCheck (Shezan Baig; D34552506)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This branch makes it so that we don't need to use the `MicrotaskSuppression`
guard whenever our product invokes Javascript.  Our product uses V8 for
non-WebKit related code.  Having to use the `MicrotaskSuppression` guard would
introduce WebKit dependency on our non-WebKit related code.


### feature/opaque-bmp (Imran Haider)
TODO


### feature/preserveSelDirection (Shezan Baig; D36076457)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This branch makes it so that when text is selected inside a contenteditable,
and then a style is applied on that text, the original selection direction
would be preserved.


### feature/printMenu (Shezan Baig)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This branch just adds a print menu item to `test_shell`.


### feature/resetCachedGDISettings (Shezan Baig; D31921707, upstream: [86891](https://bugs.webkit.org/show_bug.cgi?id=86891))

Skia caches GDI settings like ClearType, so when these settings change (for
example, after remoting to a different machine, or manually changing it in the
system settings), the app does not pick up the new setting.

This branch adds a way to reset this cache.  There is also an upstream ticket,
however, this is not so much of an issue for Chrome because Chrome just creates
new processes for each tab, so it picks up the new setting that way.


### feature/v8dll (Shezan Baig)

This is a feature we needed in our product, so it is unlikely that we will send
this upstream.

This is just a small change to use V8 in a separate DLL.


### feature/virtualAllocHooks (Lilit Darbinyan)
TODO

### feature/timeline-on-startup (Imran Haider)
TODO


## Retired branches

The following branches exist in our repository, but have been retired (either
because the changes are available in our latest upstream snapshot, or because
we simply no longer need the changes for our product).

### bugfix/flexboxOverrideHeight (Shezan Baig; D33177416)
TODO

### bugfix/indentLink (Shezan Baig; D32418391; upstream: [87428](https://bugs.webkit.org/show_bug.cgi?id=87428))
TODO

### bugfix/offsetleft (Shezan Baig; upstream: [34875](https://bugs.webkit.org/show_bug.cgi?id=34875))
TODO

### bugfix/textOriginRounding (Alex Buch; D34915731)
TODO

### bugfix/typingStyleAfterDelete (Shezan Baig; D32879084; upstream: [82401](https://bugs.webkit.org/show_bug.cgi?id=82401))
TODO

### feature/betterVirtualAllocStats (Lilit Darbinyan)
TODO

### feature/instrumentRandomVirtualAlloc (Tianyin Zhang)
TODO

---
###### Microsoft, Windows, Visual Studio and ClearType are registered trademarks of Microsoft Corp.
###### Firefox is a registered trademark of Mozilla Corp.
###### Chrome is a registered trademark of Google Inc.
