licenses(["notice"])

genrule(
    name = "gen_qrenderdoc",
    srcs = glob(["**/*"]),
    outs = [
        "librenderdoc.so",
        "qrenderdoc",
        "renderdoc_capture.json",
        "renderdoccmd",
    ],
    cmd = (
        #"CC='$(CC)' CXX='$(CC)' " +
        #"CFLAGS='$(CC_FLAGS)' CXXFLAGS='$(CC_FLAGS)' " +
        "$(location build_bazel.sh) " +
        "$$(dirname $(location :build_bazel.sh)) " +
        "$(location qrenderdoc) "
    ),
    tools = [
        ":build_bazel.sh",
    ],
)
