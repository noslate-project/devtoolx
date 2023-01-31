{
  "targets": [
    {
      "target_name": "devtoolx",
      "sources": [ 
        "src/devtoolx.cc",
        "src/library/json.hpp",
        "src/memory/heapsnapshot.cc",
        "src/memory/node.cc",
        "src/memory/edge.cc",
        "src/memory/snapshot_parser.cc",
        "src/memory/parser.cc",
        "src/memory/tarjan.cc"
      ],
      "include_dirs": [
        "src",
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        [
          "OS=='mac'",
          {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "GCC_OPTIMIZATION_LEVEL": "3",
              "GCC_ENABLE_CPP_RTTI": "YES",
              "OTHER_CFLAGS": [
                "-std=c++17",
                "-Wconversion",
                "-Wno-sign-conversion",
              ]
            },
          }
        ],
        [
          "OS=='linux'",
          {
            "cflags": [
              "-O3",
              "-std=c++17",
              "-Wno-sign-compare",
              "-Wno-cast-function-type",
            ],
          }
        ]
      ]
    }
  ]
}
