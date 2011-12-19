var ns = require('../lib/node-screen.js');

var screen = ns.createScreen(function(client) {
	console.log('new client connected');
});
