#ifndef _AXSOCKET_
#define _AXSOCKET_
#include <napi.h>

#define CALLSIGN_SIZE 15
#define MAX_DIGI 8
#define BUFFER_SIZE 1500

namespace axsocket {
  struct ax25via {
    char digi[CALLSIGN_SIZE];
    bool repeated;
  };

  struct ax25datagram {

    char from[CALLSIGN_SIZE];
    char to[CALLSIGN_SIZE];
    ax25via via[MAX_DIGI];
    char data[BUFFER_SIZE];
  };

  struct axports {
    char *portName;
    // char *deviceName;
    char *portCallsign;
    char *portDevice;
  };

  char *decodeCallsign(char *buf, char *data);

  struct axports *enumeratePorts();
  Napi::Array enumeratePortsWrap(const Napi::CallbackInfo& info);

  int createAX25Socket();
  Napi::Number createAX25SocketWrap(const Napi::CallbackInfo& info);

  bool selectReadSocket(int sock, int timeout);
  Napi::Boolean selectReadSocketWrap(const Napi::CallbackInfo& info);

  size_t readSocket(int sock, char* buffer);
  Napi::Object readSocketWrap(const Napi::CallbackInfo& info);

  void decodePacket(char *data, size_t size, ax25datagram *buffer);
  Napi::Object readAndDecodePacket(const Napi::CallbackInfo& info);

  int writeUISocket(std::string from, std::string to, std::string data, std::vector<std::string> via);
  Napi::Number writeUISocketWrap(const Napi::CallbackInfo& info);
  
  Napi::Object Init(Napi::Env env, Napi::Object exports);
}

#endif /* _AXSOCKET_ */