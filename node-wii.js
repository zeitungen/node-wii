// javascript shim that lets our object inherit from EventEmitter
var nodewii = require(__dirname + '/build/Release/nodewii.node');
var WiiMote = nodewii.WiiMote;
var events = require('events');

inherits(WiiMote, events.EventEmitter);
copy(exports,nodewii);
exports.WiiMote = WiiMote;

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

// copy object
function copy ( target, source ) {
	for ( var k in source )
		target[k] = source[k];
}