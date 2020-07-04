const UnixAX25 = require('./index');

const ax25 = new UnixAX25();
ax25.on('data', (packet) => {
  console.log(`fm ${packet.from} to ${packet.to}${packet.via.length > 0 ? ' via ' : ''}${packet.via.map(v => `${v.digi}${v.repeated ? '*' : ''}`)}`);
  console.log(packet.data);
});
ax25.startUIListener(5000);
