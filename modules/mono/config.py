# Prior to .NET Core, we supported these: ["windows", "macos", "linuxbsd", "android", "haiku", "web", "ios"]
# Eventually support for each them should be added back (except Haiku if not supported by .NET Core)
supported_platforms = ["windows", "macos", "linuxbsd"]
supported_library_platforms = ["windows", "macos", "linuxbsd", "android", "ios"]


def can_build(env, platform):
    if env["arch"].startswith("rv"):
        return False

    if env.editor_build:
        env.module_add_dependencies("mono", ["regex"])

    return True


def configure(env):
    platform = env["platform"]

    if env["library_type"] in ["static_library", "shared_library"]:
        _supported_platforms = supported_library_platforms
    else:
        _supported_platforms = supported_platforms
    if platform not in _supported_platforms:
        raise RuntimeError("This module does not currently support building for this platform")

    env.add_module_version_string("mono")


def get_doc_classes():
    return [
        "CSharpScript",
        "GodotSharp",
    ]


def get_doc_path():
    return "doc_classes"


def is_enabled():
    # The module is disabled by default. Use module_mono_enabled=yes to enable it.
    return False
