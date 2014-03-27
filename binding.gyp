{
  "targets": [
    {
      "target_name": "node-wii",
      "sources": [ "src/base.cc", "src/wiimote.cc" ],
      "libraries": [ "-lcwiid", "-lbluetooth" ]
    }
  ]
}