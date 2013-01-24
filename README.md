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

  The list of branches along with descriptions and test cases can be found
[here](http://bloomberg.github.com/chromium.bb/).


## Overall Structure

The structure of this repository is somewhat unconventional, but it serves our
purpose well.  The `master` branch is not really used (it just contains this
README).

Our real entry points are the snapshots we get from upstream.  We tag each of
these snapshots using the format `upstream/<channel>/<version>`, for example:
`upstream/stable/21.0.1180.60`.

Each tag we snapshot is committed on top of the tag for the previous version.
This essentially forms a linear branch of upstream versions.  This is the
<code>upstream/latest</code> branch.

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

* Setup your build environment, as per [these
  instructions](http://www.chromium.org/developers/how-tos/build-instructions-windows).
* Checkout one of the upstream tags (you can pick the latest version by using
  the `upstream/latest` branch).
* If you are using `upstream/stable/24.0.1312.52` or later, you will need to
  generate your projects.  Note that in previous versions, the generated
  project files were checked into the repo, so this step was not necessary.
** Open a command-prompt window and set the following environment variables:
   <code>
       set GYP_GENERATORS=msvs
       set GYP_MSVS_VERSION=2008
       set CHROMIUM_GYP_FILE=src/webkit/webkit.sln
   </code>
** Run the following command from inside the top-level `chromium/` directory:
   <code>
       gclient runhooks
   </code>
* Open `chromium/src/webkit/webkit.sln`.  This is the generated solution file
  for `test_shell`.
* Build the `test_shell` project.
* Now you can start merging branches that you're interested in (see branch
  descriptions [here](http://bloomberg.github.com/chromium.bb/)).
* Each branch introduces a specific fix, which you can test by running
  `test_shell` against the URL provided in the branch description.

---
###### Microsoft, Windows, Visual Studio and ClearType are registered trademarks of Microsoft Corp.
###### Firefox is a registered trademark of Mozilla Corp.
###### Chrome is a registered trademark of Google Inc.
