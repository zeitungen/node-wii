{
  "targets": [
    {
	"target_name": "nodewii",
	"sources": [ "src/base.cc", "src/wiimote.cc" ],
    	"libraries": [ "-lbluetooth" ],
        "dependencies": [ "./binding.gyp:cwiid" ],
        "include_dirs": [ "./external/libcwiid/" ]
    },
    {
    	"target_name": "cwiid",
    	"sources": [ "external/libcwiid/bluetooth.c", "external/libcwiid/command.c",
    				"external/libcwiid/connect.c", "external/libcwiid/interface.c",
    				"external/libcwiid/process.c", "external/libcwiid/state.c",
    				"external/libcwiid/thread.c", "external/libcwiid/util.c" ],
    	"libraries": [ "-lbluetooth", "-lpthread", "-lrt" ]
    }
  ]
}