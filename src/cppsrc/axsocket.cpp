#include "axsocket.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netax25/ax25.h>
#include <netax25/axconfig.h>
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


int axsocket::createAX25Socket() {
  int activePorts = ax25_config_load_ports();

  if (activePorts == 0) {
    fprintf(stderr, "No AX.25 port data configured\n");
    return -1;
  }
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

char *axsocket::decodeCallsign(char *buf, char *data)
{
	int i, ssid;
	char *s;
	char c;

	s = buf;

	for (i = 0; i < ALEN; i++) {
		c = (data[i] >> 1) & 0x7F;

		if (c != ' ')
			*s++ = c;
	}

	if ((ssid = (data[ALEN] & SSID)) != 0)
		sprintf(s, "-%d", ssid >> 1);
	else
		*s = '\0';

	return buf;
}

void axsocket::decodePacket(char *data, size_t size, ax25datagram *buffer) {
  int type;

  type = data[0] & 0xf;

  if (type == 0) {
    data += 1;
    size -= 1;

    char tmp[15];
    int end;
    if (size < 8) {
      printf("AX25: bad header!\n");
      return;
    }
    if (size < (AXLEN + AXLEN + 1)) {
      printf("AX25: bad header!\n");
    }
    strncpy(buffer->from, decodeCallsign(tmp, data + AXLEN), CALLSIGN_SIZE - 1);
    strncpy(buffer->to, decodeCallsign(tmp, data), CALLSIGN_SIZE - 1);
    end = (data[AXLEN + ALEN] & HDLCAEB);
    data += (AXLEN + AXLEN);
    size -= (AXLEN + AXLEN);
    if (!end) {
      int i = 0;
      while (!end) {
        strncpy(buffer->via[i].digi, decodeCallsign(tmp, data), CALLSIGN_SIZE);
        buffer->via[i].repeated = (bool) (data[ALEN] & REPEATED);
        end = (data[ALEN] & HDLCAEB);
        data += AXLEN;
        size -= AXLEN;
        i++;
      }
    }

    data += 2;
    size -= 2;
    strncpy(buffer->data, data, size > BUFFER_SIZE ? BUFFER_SIZE : size);
  }
  return;
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
  ax25datagram packet;
  memset(&packet, 0, sizeof(packet));
  decodePacket(buffer, size, &packet);

  returnValue.Set("from", Napi::String::New(env, packet.from));
  returnValue.Set("to", Napi::String::New(env, packet.to));
  returnValue.Set("data", Napi::String::New(env, packet.data));
  Napi::Array returnVia = Napi::Array::New(env);

  int i = 0;
  while (packet.via[i].digi[0] != '\0') {
    Napi::Object digi = Napi::Object::New(env);
    digi.Set("digi", Napi::String::New(env, packet.via[i].digi));
    digi.Set("repeated", Napi::Boolean::New(env, packet.via[i].repeated));
    returnVia.Set(i, digi);
    i++;
  }
  returnValue.Set("via", returnVia);

  return returnValue;
}

Napi::Object axsocket::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("createAX25Socket", Napi::Function::New(env, createAX25SocketWrap));
  exports.Set("selectReadSocket", Napi::Function::New(env, selectReadSocketWrap));
  exports.Set("readSocket", Napi::Function::New(env, readSocketWrap));
  exports.Set("readAndDecodePacket", Napi::Function::New(env, readAndDecodePacket));
  return exports;
}
