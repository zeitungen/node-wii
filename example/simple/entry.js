/*
var traverse = require('traverse');
var obj = traverse({ a : 3, b : [ 4, 5 ] }).map(function (x) {
    if (typeof x === 'number') this.update(x * 100)
});

var obj = traverse(wii);

console.log(obj);
*/

var traverse = require('traverse');
var wii = traverse(require( './nodewii.node' ));
console.dir(obj);