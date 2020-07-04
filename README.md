# unix-ax25

This package allows Node.JS applications to use the native AX25 networking in the Linux kernel. This is very much a work in progress.

## Prerequisites

1. Working AX25 setup, including configured axports file
1. libax25 headers (can usually be found in the libax25-dev package)

## Building
``` bash
npm run build
```

## Import

You can import this package as follows:

``` javascript
const UnixAX25 = require('unix-ax25');
```
or
``` javascript
import UnixAX25 from 'unix-ax25';
```

## Interface

This package creates a class that inherits from `EventEmitter`

### new UnixAX25()

The constructor requires no arguments. It will create an instance of this class and open the AX25 socket.
``` javascript
const ax25 = new UnixAX25();
```

### startUIListener(timeout)

Start listening for data on the AX25 socket. Specify an amount of time to wait before trying again in milliseconds. It will keep listening for AX25 packets until `stopUIListener()` is called.
``` javascript
ax25.startUIListener(500); // Wait half a second between read attempts
```

### stopUIListener()

Stop listening for AX25 packets
``` javascript
ax25.stopUIListener();
```

## Events

As this class inherits from `EventEmitter`, you can subscribe to events.

### data

Subscribe to the `data` event to receive a packet in the following format:
``` javascript
{
  from: String,       // The callsign and SSID sending the packet
  to: String,         // The callsign and SSID the packet is sent to
  via: [{
    digi: String,     // The callsign and SSID of specified digipeters
    repeated: Boolean // If true, the specified digipeter has repeated this packet
  }],
  data: String        // The payload of the packet
}
```
Example:
``` javascript
ax25.on('data', (packet) => {
  console.log(`Received from: ${packet.from}`);
  console.log(`Sent to: ${packet.to}`);
  console.log(`Payload: ${packet.data}`);
});
```

## Example

A quick example similar to the `listen` utility from the `ax25-apps` package can be found in `src/listen.js`.
``` bash
node src/listen.js
```
