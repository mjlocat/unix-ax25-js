#include "axsocket.h"
#include "ax25structures.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netax25/ax25.h>
#include <netax25/axconfig.h>
#include <netax25/axlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef __GLIBC__
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif /* __GLIBC__ */

#define AXLEN 7
#define ALEN 6
#define SSID 0x1E
#define HDLCAEB 0x01
#define REPEATED 0x80
#define USEC 1000000

std::vector<axsocket::axport> axsocket::enumeratePorts() {
  std::vector<axsocket::axport> listOfPorts;
  char *portName = NULL, *portCallsign, *portDevice;
  int activePorts = ax25_config_load_ports();
  if (activePorts > 0) {
    for (int i = 0; i < activePorts; i++) {
      portName = ax25_config_get_next(portName);
      if (portName == NULL) {
        break;
      }
      portCallsign = ax25_config_get_addr(portName);
      portDevice = ax25_config_get_dev(portName);
      axsocket::axport port(portName, portCallsign, portDevice);
      listOfPorts.push_back(port);
    }
  }
  return listOfPorts;
}

Napi::Array axsocket::enumeratePortsWrap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Array portsArray = Napi::Array::New(env);

  std::vector<axsocket::axport> listOfPorts = axsocket::enumeratePorts();
  for (unsigned int i = 0; i < listOfPorts.size(); i++) {
    Napi::Object port = Napi::Object::New(env);
    port.Set("portName", Napi::String::New(env, listOfPorts[i].getPortName()));
    port.Set("portCallsign", Napi::String::New(env, listOfPorts[i].getPortCallsign()));
    port.Set("portDevice", Napi::String::New(env, listOfPorts[i].getPortDevice()));
    portsArray.Set(i, port);
  }
  return portsArray;
}

int axsocket::createAX25Socket() {
  int sock = socket(AF_PACKET, SOCK_PACKET, htons(ETH_P_AX25));
  if (sock == -1) {
    perror("socket");
  }
  return sock;
}

Napi::Number axsocket::createAX25SocketWrap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Number sock = Napi::Number::New(env, axsocket::createAX25Socket());
  return sock;
}

bool axsocket::selectReadSocket(int sock, int timeout = 0) {
  timeval tv = { (timeout / USEC), (timeout % USEC) };
  fd_set rfds;
  int status;

  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);
  status = select(sock + 1, &rfds, NULL, NULL, &tv);
  return (status > 0);
}

Napi::Boolean axsocket::selectReadSocketWrap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Socket file handle expected").ThrowAsJavaScriptException();
  }
  Napi::Number sock = info[0].As<Napi::Number>();
  Napi::Number timeout = info.Length() >= 2 ? info[1].As<Napi::Number>() : Napi::Number::New(env, 0);
  return Napi::Boolean::New(env, selectReadSocket(sock.Int32Value(), timeout.Int32Value()));
}


size_t axsocket::readSocket(int sock, char* buffer) {
  struct sockaddr sa;
  socklen_t asize = sizeof(sa);
  size_t size;

  size = recvfrom(sock, buffer, BUFFER_SIZE, 0, &sa, &asize);
  if (size == (size_t) -1) {
    fprintf(stderr, "No data received\n");
  }

  return size;
}

Napi::Object axsocket::readSocketWrap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }

  Napi::Object returnValue = Napi::Object::New(env);
  Napi::Number sock = info[0].As<Napi::Number>();
  char buffer[BUFFER_SIZE];
  size_t size = axsocket::readSocket(sock.Int32Value(), buffer);
  returnValue.Set("size", Napi::Number::New(env, size));
  returnValue.Set("raw", Napi::String::New(env, buffer, size));
  return returnValue;
}

axsocket::ax25datagram axsocket::decodePacket(char *data, size_t size) {
  axsocket::ax25datagram packet;
  int type;

  type = data[0] & 0xf;

  if (type == 0) {
    data += 1;
    size -= 1;

    int end;
    if (size < 8) {
      printf("AX25: bad header!\n");
      throw;
    }
    if (size < (AXLEN + AXLEN + 1)) {
      printf("AX25: bad header!\n");
      throw;
    }

    ax25_address *from = (ax25_address *) (data + AXLEN);
    ax25_address *to = (ax25_address *) (data + 0);
    packet.setFrom(ax25_ntoa(from));
    packet.setTo(ax25_ntoa(to));
    end = (data[AXLEN + ALEN] & HDLCAEB);
    data += (AXLEN + AXLEN);
    size -= (AXLEN + AXLEN);
    if (!end) {
      int i = 0;
      while (!end) {
        ax25_address *digi = (ax25_address *) (data + 0);
        char *digiCall = ax25_ntoa(digi);
        bool repeated = (data[ALEN] & REPEATED);
        packet.addDigi(digiCall, repeated);

        end = (data[ALEN] & HDLCAEB);
        data += AXLEN;
        size -= AXLEN;
        i++;
        if (i >= MAX_DIGI) {
          break;
        }
      }
    }

    data += 2;
    size -= 2;
    data[size] = '\0';
    packet.setData(data);
  }
  return packet;
}

Napi::Object axsocket::readAndDecodePacket(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Socket file handle expected").ThrowAsJavaScriptException();
  }

  Napi::Object returnValue = Napi::Object::New(env);
  Napi::Number sock = info[0].As<Napi::Number>();
  char buffer[BUFFER_SIZE];
  size_t size = axsocket::readSocket(sock.Int32Value(), buffer);
  axsocket::ax25datagram packet = decodePacket(buffer, size);

  returnValue.Set("from", Napi::String::New(env, packet.getFrom()));
  returnValue.Set("to", Napi::String::New(env, packet.getTo()));
  returnValue.Set("data", Napi::String::New(env, packet.getData()));

  Napi::Array returnVia = Napi::Array::New(env);
  std::vector<axsocket::ax25via> digis = packet.getDigis();
  for (unsigned int i = 0; i < digis.size(); i++) {
    Napi::Object digi = Napi::Object::New(env);
    digi.Set("digi", Napi::String::New(env, digis[i].getDigiCallsign()));
    digi.Set("repeated", Napi::Boolean::New(env, digis[i].digiDidRepeat()));
    returnVia.Set(i, digi);
  }
  returnValue.Set("via", returnVia);

  return returnValue;
}

std::string axsocket::buildDestStr(axsocket::ax25datagram packet) {
  std::string destStr = packet.getTo();
  std::vector<axsocket::ax25via> digis = packet.getDigis();

  for (unsigned int i = 0; i < digis.size(); i++) {
    destStr += " " + digis[i].getDigiCallsign();
  }

  return destStr;
}

ssize_t axsocket::writeUISocket(axsocket::ax25datagram packet) {
  struct full_sockaddr_ax25 dest;
  struct full_sockaddr_ax25 src;
  std::string destStr = axsocket::buildDestStr(packet);
  int destlen, srclen, sock;
  ssize_t wrote;

  if ((destlen = ax25_aton(destStr.c_str(), &dest)) == -1) {
    std::cerr << "Unable to convert destination callsign \"" << destStr << "\"\n";
    return -1;
  }

  if ((srclen = ax25_aton(packet.getFrom().c_str(), &src)) == -1) {
    std::cerr << "Unable to convert source callsign \"" << packet.getFrom() << "\"\n";
    return -1;
  }

  if ((sock = socket(AF_AX25, SOCK_DGRAM, 0)) == -1) {
    perror("Unable to open socket");
    return -1;
  }

  if (bind(sock, (struct sockaddr *) &src, srclen) == -1) {
    perror("Unable to bind to socket");
    return -1;
  }

  std::string data = packet.getData();
  if ((wrote = sendto(sock, data.c_str(), data.length(), 0, (struct sockaddr *) &dest, destlen)) == -1) {
    perror("Unable to send data");
    return -1;
  }

  return wrote;
}

Napi::Number axsocket::writeUISocketWrap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int writeResult;
  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Expected at least 3 arguments").ThrowAsJavaScriptException();
  }
  if (!info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
    Napi::TypeError::New(env, "Invalid parameter types passed").ThrowAsJavaScriptException();
  }
  axsocket::ax25datagram packet;
  packet.setFrom(info[0].As<Napi::String>());
  packet.setTo(info[1].As<Napi::String>());
  packet.setData(info[2].As<Napi::String>());
  if (info.Length() >= 4) {
    if (!info[3].IsArray()) {
      Napi::TypeError::New(env, "Expected via parameter to be an array").ThrowAsJavaScriptException();
    }
    Napi::Array tmpVia = info[3].As<Napi::Array>();
    int tmpLen = std::min<int>(MAX_DIGI, tmpVia.Length());
    for (int i = 0; i < tmpLen; i++) {
      Napi::Value tmpVal = tmpVia.Get(i);
      if (!tmpVal.IsString()) {
        Napi::TypeError::New(env, "Expected via to be an array of strings").ThrowAsJavaScriptException();
      }
      packet.addDigi(tmpVal.ToString());
    }
  }
  writeResult = writeUISocket(packet);
  return Napi::Number::New(env, writeResult);
}

Napi::Object axsocket::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("createAX25Socket", Napi::Function::New(env, createAX25SocketWrap));
  exports.Set("selectReadSocket", Napi::Function::New(env, selectReadSocketWrap));
  exports.Set("readSocket", Napi::Function::New(env, readSocketWrap));
  exports.Set("readAndDecodePacket", Napi::Function::New(env, readAndDecodePacket));
  exports.Set("writeUISocket", Napi::Function::New(env, writeUISocketWrap));
  exports.Set("enumeratePorts", Napi::Function::New(env, enumeratePortsWrap));
  return exports;
}
