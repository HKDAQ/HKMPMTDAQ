#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <MPMTDataChunk.h>
#include <Logger.h>
#include <Store.h>

class DataManager{

 public:

  DataManager(zmq::socket_t &insock, Logger &inlogger, Store &invariables);
  bool GetData();
  bool ManageQueues();
  bool Receive();
  bool Send();
  void UpdateStatus();

 private:

  zmq::socket_t* sock;
  Store *variables;
  std::string UUID;
  std::deque<MPMTDataChunk*> data_queue;
  std::deque<MPMTDataChunk*> sent_queue;
  Logger* logger;
  unsigned int data_id;
  int data_timeout;
  boost::posix_time::time_duration resend_period;
  int resend_attempts;
  int queue_warning_limit;
  int queue_max_size;
  long data_chunk_size_ms;
  boost::posix_time::ptime last_get;

  int fake_data_rate;
};


#endif
