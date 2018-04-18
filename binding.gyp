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
        "src/memory/parser.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        [
          "OS=='mac'",
          {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "GCC_ENABLE_CPP_RTTI": "YES",
              "MACOSX_DEPLOYMENT_TARGET": "10.9",
              "CLANG_CXX_LANGUAGE_STANDARD": "c++11"
            }
          }
        ]
      ]
    }
  ]
}
