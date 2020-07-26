#include "ax25structures.h"
#include <string>

axsocket::axport::axport(char *portName, char *portCallsign, char *portDevice) {
  this->portName = portName;
  this->portCallsign = portCallsign;
  this->portDevice = portDevice;
}

std::string axsocket::axport::getPortName() {
  return this->portName;
}

std::string axsocket::axport::getPortCallsign() {
  return this->portCallsign;
}

std::string axsocket::axport::getPortDevice() {
  return this->portDevice;
}

axsocket::ax25via::ax25via(std::string digi, bool repeated) {
  this->digi = digi;
  this->repeated = repeated;
}

std::string axsocket::ax25via::getDigiCallsign() {
  return this->digi;
}

bool axsocket::ax25via::digiDidRepeat() {
  return this->repeated;
}

axsocket::ax25datagram::ax25datagram() {

}

axsocket::ax25datagram::~ax25datagram() {
  // for (unsigned int i = 0; i < this->via.size(); i++) {
  //   delete this->via[i];
  // }
}

void axsocket::ax25datagram::setFrom(char *from) {
  this->from = from;
}

void axsocket::ax25datagram::setTo(char *to) {
  this->to = to;
}

void axsocket::ax25datagram::addDigi(char *digi, bool repeated) {
  ax25via newDigi(digi, repeated);
  this->via.push_back(newDigi);
}

void axsocket::ax25datagram::setData(char *data) {
  this->data = data;
}

void axsocket::ax25datagram::setFrom(std::string from) {
  this->from = from;
}

void axsocket::ax25datagram::setTo(std::string to) {
  this->to = to;
}

void axsocket::ax25datagram::addDigi(std::string digi, bool repeated) {
  ax25via newDigi(digi, repeated);
  this->via.push_back(newDigi);
}

void axsocket::ax25datagram::setData(std::string data) {
  this->data = data;
}

std::string axsocket::ax25datagram::getFrom() {
  return this->from;
}

std::string axsocket::ax25datagram::getTo() {
  return this->to;
}

std::vector<axsocket::ax25via> axsocket::ax25datagram::getDigis() {
  return this->via;
}

std::string axsocket::ax25datagram::getData() {
  return this->data;
}

std::string axsocket::ax25datagram::dumpPacket() {
  std::string buildString = "";
  buildString += "From: " + this->from + "\n";
  buildString += "To: " + this->to + "\n";
  for (unsigned int i = 0; i < this->via.size(); i++) {
    if (i == 0) {
      buildString += "Via:";
    }
    buildString += " " + this->via[i].getDigiCallsign() + (this->via[i].digiDidRepeat() ? "*" : "");
    if (i == this->via.size()-1) {
      buildString += "\n";
    }
  }
  buildString += "Data: " + this->data + "\n";
  return buildString;
}
