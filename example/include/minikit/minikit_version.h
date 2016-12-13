// generated file -- DO NOT EDIT
#ifndef INCLUDED_GENERATED_MINIKIT_VERSION
#define INCLUDED_GENERATED_MINIKIT_VERSION

#define CHROMIUM_VERSION "48.0.2564.103"
#define BB_PATCH_VERSION "bb0"

namespace minikit {

struct Version {
    static const char* d_chromiumVersion;
    static const char* d_bbPatchVersion;
    MINIKIT_EXPORT static const char* version_48_0_2564_103_bb0();
};  // Version

// Force linker to pull in this component's object file.
namespace {
    extern const char* (*minikit_version_assertion)() = 
        &Version::version_48_0_2564_103_bb0;
}

}  // minikit

#endif  // INCLUDED_GENERATED_MINIKIT_VERSION
