vars = {
     "chromium_git": "https://chromium.googlesource.com",
}

deps = {
    # chromium's buildtools containing libc++, libc++abi, clang_format and gn.
    "puyoai/buildtools": "https://chromium.googlesource.com/chromium/buildtools@24ebce4578745db15274e180da1938ebc1358243",

    # gflags 2.2.1
    "puyoai/third_party/gflags/gflags": "https://github.com/gflags/gflags.git@46f73f88b18aee341538c0dfc22b1710a6abedef",

    # google-glog
    "puyoai/third_party/glog/glog": "https://github.com/google/glog.git@2063b387080c1e7adffd33ca07adff0eb346ff1a",

    # googletest
    "puyoai/third_party/gtest/gtest": Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + '145d05750b15324899473340c8dd5af50d125d33',

    # jsoncpp
    # "puyoai/third_party/jsoncpp/source": Var("chromium_git") + '/external/github.com/open-source-parsers/jsoncpp.git@f572e8e42e22cfcf5ab0aea26574f408943edfa4', # from svn 248

    # tinytoml
    "puyoai/third_party/tinytoml/tinytoml": "https://github.com/mayah/tinytoml.git@3ba7a885fa725f3c0ec7d3dfbe01ddb7e35d7fb6",
}

hooks = [
    {
        "name": "gn_win",
        "pattern": ".",
        "action": [ "download_from_google_storage",
                    "--no_resume",
                    "--platform=win32",
                    "--no_auth",
                    "--bucket", "chromium-gn",
                    "-s", "puyoai/buildtools/win/gn.exe.sha1",
        ],
    },
    {
        "name": "gn_mac",
        "pattern": ".",
        "action": [ "download_from_google_storage",
                    "--no_resume",
                    "--platform=darwin",
                    "--no_auth",
                    "--bucket", "chromium-gn",
                    "-s", "puyoai/buildtools/mac/gn.sha1",
        ],
    },
    {
        "name": "gn_linux64",
        "pattern": ".",
        "action": [ "download_from_google_storage",
                    "--no_resume",
                    "--platform=linux*",
                    "--no_auth",
                    "--bucket", "chromium-gn",
                    "-s", "puyoai/buildtools/linux64/gn.sha1",
        ],
    },
    # Pull clang-format binaries using checked-in hashes.
    {
        'name': 'clang_format_win',
        'pattern': '.',
        'condition': 'host_os == "win"',
        'action': [ 'download_from_google_storage',
                    '--no_resume',
                    '--no_auth',
                    '--bucket', 'chromium-clang-format',
                    '-s', 'puyoai/buildtools/win/clang-format.exe.sha1',
        ],
    },
    {
        'name': 'clang_format_mac',
        'pattern': '.',
        'condition': 'host_os == "mac"',
        'action': [ 'download_from_google_storage',
                    '--no_resume',
                    '--no_auth',
                    '--bucket', 'chromium-clang-format',
                    '-s', 'puyoai/buildtools/mac/clang-format.sha1',
        ],
    },
    {
        'name': 'clang_format_linux',
        'pattern': '.',
        'condition': 'host_os == "linux"',
        'action': [ 'download_from_google_storage',
                    '--no_resume',
                    '--no_auth',
                    '--bucket', 'chromium-clang-format',
                    '-s', 'puyoai/buildtools/linux64/clang-format.sha1',
        ],
    },
]

