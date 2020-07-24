const UnixAX25 = require('./index');

const ax25 = new UnixAX25();
ax25.writeUISocket({
  from: 'KJ6LNH',
  to: 'TEST',
  data: '=3800.63N/12158.15W[207/005/A=000347',
  via: ['WIDE1-1']
});

