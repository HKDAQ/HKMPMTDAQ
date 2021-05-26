#include <MPMTDataChunk.h>

MPMTDataChunk::MPMTDataChunk(std::string inUUID){

  UUID=inUUID;
  data_id=0;
  size=0;
  attempts=0;
  last_send=boost::posix_time::microsec_clock::universal_time();
  in_use=false;
}

bool MPMTDataChunk::Send(zmq::socket_t* sock){

  data_id++;
  attempts++;

  zmq::message_t msg1(&data_id,sizeof data_id, NULL);
  sock->send(msg1,ZMQ_SNDMORE);  

  size=hits.size();
  zmq::message_t msg2(&size,sizeof size, NULL);
  sock->send(msg2,ZMQ_SNDMORE);

  if(size){
    zmq::message_t msg3(&(hits.at(0)), (hits.at(0).Size())*hits.size(), NULL);
    sock->send(msg3,ZMQ_SNDMORE);    

  }

  zmq::message_t msg4(&UUID,sizeof UUID, NULL);   
  sock->send(msg4);          

  return true;

}

bool MPMTDataChunk::Receive(zmq::socket_t* sock){


  return true;
}      
