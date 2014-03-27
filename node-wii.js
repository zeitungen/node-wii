// javascript shim that lets our object inherit from EventEmitter
var WiiMote = require(__dirname + '/build/Release/nodewii.node').WiiMote;
var events = require('events');

inherits(WiiMote, events.EventEmitter);
exports.WiiMote = WiiMote;

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}