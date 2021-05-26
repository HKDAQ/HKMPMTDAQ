#include <DataManager.h>

DataManager::DataManager(zmq::socket_t &insock, Logger &inlogger, Store &variables){

  sock=&insock;
  logger=&inlogger;

  std::string data_sock_port;
  int data_timeout;
  int resend_time;

  variables.Get("data_sock_port",data_sock_port);
  variables.Get("data_timeout",data_timeout);

  variables.Get("UUID", UUID);

  variables.Get("data_timeout", data_timeout);
  variables.Get("resend_period", resend_time);
  resend_period=boost::posix_time::microseconds(resend_time);
  variables.Get("resend_attempts", resend_attempts);
  variables.Get("queue_warning_limit", queue_warning_limit);
  variables.Get("queue_max_size", queue_max_size);
  variables.Get("data_chunk_size_ms", data_chunk_size_ms);

  sock->bind(data_sock_port.c_str());
  sock->setsockopt(ZMQ_RCVTIMEO, data_timeout);
  sock->setsockopt(ZMQ_SNDTIMEO, data_timeout);


  variables.Get("fake_data_rate", fake_data_rate);
}


bool DataManager::GetData(){

  boost::posix_time::time_duration lapse=boost::posix_time::microsec_clock::universal_time()-last_get;
  long ms= lapse.total_microseconds();

  if(ms>=data_chunk_size_ms){

    MPMTDataChunk* data=new MPMTDataChunk(UUID);

    int numhits=(fake_data_rate/1000)*ms;
    data->hits.resize(numhits);

    for(int i=0; i<numhits; i++){
      
      data->hits.at(i).adc_charge= 1000+i; 
      data->hits.at(i).pmt_id= i%19;
      data->hits.at(i).time_corse= i;
      data->hits.at(i).time_fine= i*100;

    }

    data_queue.push_back(data);

  }

  return true;

}

bool DataManager::ManageQueues(){

  if((data_queue.size()+sent_queue.size())> queue_max_size){

    logger->Send("Warnning!!! MPMT buffer full oldest data is being deleted");

    if(data_queue.size()){

      if(!data_queue.at(0)->in_use) delete data_queue.at(0);
      else data_queue.at(0)->in_use=false;
      data_queue.at(0)=0;
      data_queue.pop_front();

    }

  }
  else if((data_queue.size()+sent_queue.size())> queue_warning_limit) logger->Send("Warnning!!! MPMT buffer nearly full");


  for(std::deque<MPMTDataChunk*>::iterator it=sent_queue.begin(); it!=sent_queue.end(); it++){

    boost::posix_time::time_duration lapse=resend_period-(boost::posix_time::microsec_clock::universal_time() - (*it)->last_send);
    
    if(lapse.is_negative()){
      if((*it)->attempts< resend_attempts){
	(*it)->attempts++;
	data_queue.push_front((*it));
      }
      else{
	if(!(*it)->in_use) delete (*it);
	else (*it)->in_use=false;
      }
	(*it)=0;
	sent_queue.erase(it);
	break;
      }
  }

  return true;

}

bool DataManager::Send(){

  bool ret=true;

  if(data_queue.size()){
    data_queue.at(0)->in_use=true;
    data_id++;
    data_queue.at(0)->data_id=data_id;
    data_queue.at(0)->last_send=boost::posix_time::microsec_clock::universal_time();
    ret*=data_queue.at(0)->Send(sock);
    sent_queue.push_back(data_queue.at(0));
    data_queue.at(0)=0;
    data_queue.pop_front();
  }

  if(!ret) logger->Send("WARNING!!! MPMT error sending data");
  
  return ret;
}

bool DataManager::Receive(){

  bool found=true;
  
  Store akn;

  if(akn.Receive(sock)){
    
    unsigned long received_id=0;
    if(akn.Get("data_id",received_id))
      
      for(std::deque<MPMTDataChunk*>::iterator it=sent_queue.begin(); it!=sent_queue.end(); it++){
	
	if((*it)->data_id == received_id){
	  if(!(*it)->in_use) delete (*it);
	  else (*it)->in_use=false;
	  (*it)=0;
	  sent_queue.erase(it);
	  found=true;
	  break;
	}
	
      }
    
    if(!found){
      for(std::deque<MPMTDataChunk*>::iterator it=data_queue.begin(); it!=data_queue.end(); it++){
	
	if((*it)->data_id == received_id){ 
	  if(!(*it)->in_use) delete (*it);
	  else (*it)->in_use=false;
	  (*it)=0;
	  data_queue.erase(it);
	  break;
	}
	
      }
      
      return true; 
   
    }
    
    logger->Send("Warning!!! MPMT data_id missing from data AKN");
    return false;
    
  }
  
  logger->Send("Warning!!! MPMT failed to receive data AKN");
  return false;
}



