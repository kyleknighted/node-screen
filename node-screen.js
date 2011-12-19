var binding = require('./build/Release/binding');
var events = require('events');

function Screen(/* [ options, ] listener */) {
  if (!(this instanceof Screen)) return new Screen(arguments[0], arguments[1]);
  events.EventEmitter.call(this);

  var options;

  if (typeof arguments[0] == 'function') {
    options = {};
    this.on('connection', arguments[0]);
  } else {
    options = arguments[0] || {};

    if (typeof arguments[1] == 'function') {
      this.on('connection', arguments[1]);
    }
  }

  this.width = options.width || 800;
  this.height = options.height || 600;
  this._screen = new binding.RfbScreen(this.width, this.height);

  this._screen.on('client', function() {
 	this.emit('onconnection');
  });
}
util.inherits(Server, events.EventEmitter);
exports.Screen = Screen;

Screen.prototype.close = function() {
	this._screen.close();
};

Screen.prototype.listen = function(port, host) {
	this._screen.listen(port || 5900, host || '0.0.0.0');
};

exports.createScreen = function(options, listener) {
	return new Screen(arguments[0], arguments[1]);
};
