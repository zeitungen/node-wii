{
  "targets": [
    {
    "target_name": "nodewii",
    "sources": [ "src/base.cc", "src/wiimote.cc" ],
        "libraries": [ "-lbluetooth" ],
        "dependencies": [ "./binding.gyp:cwiid" ],
        "include_dirs": [ "./external/libcwiid/", "./include/" ]
    },
    {
        "target_name": "cwiid",
        "type": "static_library",
        "sources": [ "external/libcwiid/bluetooth.c", "external/libcwiid/command.c",
                    "external/libcwiid/connect.c", "external/libcwiid/interface.c",
                    "external/libcwiid/process.c", "external/libcwiid/state.c",
                    "external/libcwiid/thread.c", "external/libcwiid/util.c" ],
        "libraries": [ "-lbluetooth", "-lpthread", "-lrt" ],
        "include_dirs": [ "./external/libcwiid/" ],
        "direct_dependent_settings": {
            "include_dirs": [ "./external/libcwiid/" ]
        }
    }
  ]
}