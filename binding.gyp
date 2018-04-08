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
      ]
    }
  ]
}
