#ifndef MONITOR_H
#define MONITOR_H

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Store.h>

class Monitor{

 public:

  Monitor(zmq::socket_t &insock, Store &variables);
  bool Send(int state);
  void CollectData();
  std::string Status();

 private:

  zmq::socket_t* sock;
  Store monitoring_data;


};


#endif
