#ifndef _AXSOCKET_
#define _AXSOCKET_
#include "ax25structures.h"
#include <napi.h>

#define CALLSIGN_SIZE 15
#define MAX_DIGI 8
#define BUFFER_SIZE 1500

namespace axsocket {
  std::vector<axport> enumeratePorts();
  Napi::Array enumeratePortsWrap(const Napi::CallbackInfo& info);

  int createAX25Socket();
  Napi::Number createAX25SocketWrap(const Napi::CallbackInfo& info);

  bool selectReadSocket(int sock, int timeout);
  Napi::Boolean selectReadSocketWrap(const Napi::CallbackInfo& info);

  size_t readSocket(int sock, char* buffer);
  Napi::Object readSocketWrap(const Napi::CallbackInfo& info);

  ax25datagram decodePacket(char *data, size_t size);
  Napi::Object readAndDecodePacket(const Napi::CallbackInfo& info);

  std::string buildDestStr(axsocket::ax25datagram packet);
  
  ssize_t writeUISocket(ax25datagram packet);
  Napi::Number writeUISocketWrap(const Napi::CallbackInfo& info);
  
  Napi::Object Init(Napi::Env env, Napi::Object exports);
}

#endif /* _AXSOCKET_ */