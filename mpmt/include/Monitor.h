#ifndef MONITOR_H
#define MONITOR_H

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Store.h>

class Monitor{

 public:

  Monitor(zmq::socket_t &insock, Store &invariables);
  bool Send();
  void CollectData();
 
 private:

  zmq::socket_t* sock;
  Store monitoring_data;
  Store* variables;

};


#endif
