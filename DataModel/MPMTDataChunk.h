#ifndef MPMTDATACHUNK_H
#define MPMTDATACHUNK_H

#include <vector>

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>  

#include <Hit.h>

class MPMTDataChunk{

 public:

  MPMTDataChunk(std::string inUUID="");
  bool Send(zmq::socket_t* sock);
  bool Receive(zmq::socket_t* sock);
    
  std::vector<Hit> hits;
  std::string UUID;
  unsigned long data_id;

  boost::posix_time::ptime last_send;
  int attempts;
  bool in_use;

 private:

  int size;



};



#endif
