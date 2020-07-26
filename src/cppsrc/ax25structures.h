#ifndef _AX25STRUCTURES_
#define _AX25STRUCTURES_
#include <string>
#include <vector>

namespace axsocket {
  class axport {
    private:
      std::string portName;
      std::string portCallsign;
      std::string portDevice;
    public:
      axport(char *portName, char *portCallsign, char *portDevice);
      std::string getPortName();
      std::string getPortCallsign();
      std::string getPortDevice();
  };

  class ax25via {
    private:
      std::string digi;
      bool repeated;
    public:
      ax25via(std::string digi, bool repeated);
      std::string getDigiCallsign();
      bool digiDidRepeat();
  };

  class ax25datagram {
    private:
      std::string from;
      std::string to;
      std::vector<ax25via> via;
      std::string data;
    public:
      ax25datagram();
      ~ax25datagram();
      void setFrom(char *from);
      void setTo(char *to);
      void addDigi(char *digi, bool repeated = false);
      void setData(char *data);
      void setFrom(std::string from);
      void setTo(std::string to);
      void addDigi(std::string digi, bool repeated = false);
      void setData(std::string data);
      std::string getFrom();
      std::string getTo();
      std::vector<ax25via> getDigis();
      std::string getData();
      std::string dumpPacket();
  };
}


#endif /* _AX25STRUCTURES_ */
