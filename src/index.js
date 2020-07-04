const EventEmitter = require('events');
const axsocket = require('../build/Release/axsocket.node');

class UnixAX25 extends EventEmitter {
  constructor() {
    super();
    this.socket = axsocket.createAX25Socket();
    this.selectTimeout = 0;
    this.listening = false;
    this.UIlisten = this.UIlisten.bind(this);
  }

  startUIListener(timeout = 0) {
    this.selectTimeout = timeout * 1000; // NB: C function takes microseconds, but we pass milliseconds
    this.listening = true;
    this.UIlisten();
  }

  stopUIListener() {
    this.listening = false;
  }

  UIlisten() {
    if (axsocket.selectReadSocket(this.socket, this.selectTimeout)) {
      const packet = axsocket.readAndDecodePacket(this.socket);
      this.emit('data', packet);
    }
    if (this.listening) {
      process.nextTick(this.UIlisten);
    }
  }
}

module.exports = UnixAX25;
// module.exports = {
//   createAX25Socket: axsocket.createAX25Socket,
//   readAndDecodePacket: axsocket.readAndDecodePacket
// };
