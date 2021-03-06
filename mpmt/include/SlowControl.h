#ifndef SLOWCONTROL_H
#define SLOWCONTROL_H

#include <string>

#include <Logger.h>

#include <zmq.hpp>

class SlowControl{

 public:

  SlowControl(zmq::socket_t &insock, Logger &inlogger, Store &invariables);
  std::string Command(std::string command);
  bool Config ();
  bool Send();
  bool Receive();
  bool Request();


 private:

  zmq::socket_t* sock;
  std::deque<std::vector<zmq::message_t*> > msg_queue;
  Logger* logger;
  std::string UUID;
  Store configuration_variables;
  Store* variables;
};




#endif

