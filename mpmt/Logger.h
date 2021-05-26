#ifndef LOGGER_H
#define LOGGER_H

#include <vector>
#include <string>

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Store.h>

class Logger{

 public:

  Logger(zmq::socket_t &insock, Store &variables);
  void Send(std::string message);
  
 private:
  
  zmq::socket_t* sock;
  std::vector<std::string> messages;
  boost::posix_time::time_duration period;
  boost::posix_time::ptime last_message;
  Store JSONmsg;

};



#endif
