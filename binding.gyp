{
    "targets": [{
        "target_name": "node-refprop-napi",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "sources": [
            "cppsrc/addon.cpp",
            "cppsrc/refprop.cpp",
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'libraries': [],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    },
    {
        "target_name": "action_after_build",
        "type": "none",
        "dependencies": ["node-refprop-napi"],
        "copies": [
            {
                "files": ["<(PRODUCT_DIR)/node-refprop-napi.node"],
                "destination": "."
            }
        ]
    }
    ]
}