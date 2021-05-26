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
  
  zmq::message_t msg1(&data_id,sizeof(data_id), NULL);
  sock->send(msg1,ZMQ_SNDMORE);  
  
  size=hits.size();
  zmq::message_t msg2(&size,sizeof(size), NULL);
  sock->send(msg2,ZMQ_SNDMORE);
  
  if(size){
    zmq::message_t msg3(&(hits.at(0)), (hits.at(0).Size())*hits.size(), NULL);
    sock->send(msg3,ZMQ_SNDMORE);    
    
  }

  zmq::message_t msg4(&(UUID[0]),sizeof(char)*UUID.length()+1, NULL);   
  sock->send(msg4);          
  
  return true;
  
}

bool MPMTDataChunk::Receive(zmq::socket_t* sock){
  
  
  zmq::message_t msg;
  
  if(sock->recv(&msg)){

    data_id=*(reinterpret_cast<unsigned long*>(msg.data()));

    if(msg.more() && sock->recv(&msg)){

      size=*(reinterpret_cast<int*>(msg.data()));  
     
      if(size){
	if(msg.more() && sock->recv(&msg)){

	  hits.resize(msg.size()/sizeof(Hit::Size()));
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
