{
  "targets": [{
    "target_name": "axsocket",
    "cflags!": [ "-fno-exceptions", "-lax25" ],
    "cflags_cc!": [ "-fno-exceptions", "-lax25" ],
    "sources": [
      "src/cppsrc/main.cpp",
      "src/cppsrc/axsocket.cpp"
    ],
    'include_dirs': [
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    'libraries': [
      "-lax25"
    ],
    'dependencies': [
      "<!@(node -p \"require('node-addon-api').gyp\")"
    ],
    'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
  }]
}