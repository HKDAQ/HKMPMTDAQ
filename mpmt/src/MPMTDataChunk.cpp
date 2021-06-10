#include <MPMTDataChunk.h>

MPMTDataChunk::MPMTDataChunk(std::string inUUID){

  UUID=inUUID;
  data_id=0;
  size=0;
  attempts=0;
  last_send=boost::posix_time::microsec_clock::universal_time();
 
}

bool MPMTDataChunk::Send(zmq::socket_t* sock){

  attempts++;

  sock->send(&data_id,sizeof(data_id),ZMQ_SNDMORE);
  
  size=hits.size();
  sock->send(&size,sizeof(size),ZMQ_SNDMORE);

  if(size)  sock->send(&(hits.at(0)), (hits.at(0).Size())*hits.size(),ZMQ_SNDMORE);
          
  sock->send(&(UUID[0]),sizeof(char)*UUID.length()+1);
    
  return true;
  
}

bool MPMTDataChunk::Receive(zmq::socket_t* sock){
  
  zmq::message_t msg;
  
  if(sock->recv(&msg)){

    data_id=*(reinterpret_cast<unsigned int*>(msg.data()));

    if(msg.more() && sock->recv(&msg)){

      size=*(reinterpret_cast<int*>(msg.data()));  
     
      if(size){
	if(msg.more() && sock->recv(&msg)){

	  hits.resize(size);
	  std::memcpy(&hits[0], msg.data(), msg.size());
	
	}
      }
      
      if(msg.more() && sock->recv(&msg)){
	
	UUID.resize(msg.size()/sizeof(char));
	std::memcpy(&(UUID[0]), msg.data(), msg.size());
	
	if(msg.more()) return false;
	
      }
      else return false;
      
    }
    else return false;
  }
  else return false;

  return true;
}      

