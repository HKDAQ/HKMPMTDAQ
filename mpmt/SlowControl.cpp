#include <SlowControl.h>


SlowControl::SlowControl(zmq::socket_t &insock, std::string inUUID, Logger &inlogger){

  sock=&insock;
  UUID=inUUID;
  logger=&inlogger;

}


bool SlowControl::Send(){

  bool ret=true;

  if(msg_queue.size()>0){
    if(msg_queue.at(0).size()>0){

      
      for(int i=0; i<msg_queue.at(0).size()-1; i++){
	ret*=sock->send(*(msg_queue.at(0).at(i)),ZMQ_SNDMORE);
      }
      
      ret*=sock->send(*(msg_queue.at(0).at(msg_queue.at(0).size()-1)));  

    }
    
    msg_queue.pop_front();
      
    
  }

  if(!ret){
    logger->Send("Warnning!!! Slow control reply not sending correctly");
    return false;
  }
  
  return true;

}


bool SlowControl::Receive(int &state){

  zmq::message_t* identity= new zmq::message_t;
  if(sock->recv(identity)){
    
    zmq::message_t throwaway;
    if(identity->more() && sock->recv(&throwaway)){
      
      if(throwaway.more() && configuration_variables.Receive(sock)){
	
	Store ret;
	
	ret.Set("msg_type","Command Reply"); 
	
	std::string type="";
	if(configuration_variables.Get("msg_type",type)){

	  if(type=="Config"){
	    ret.Set("AKN",Config());
	    *ret["msg_value"]="Received Config";
	    if(state==1) state=2;
	  }
	  else if(type=="Command"){
	    
	    std::string command="";
	    if(configuration_variables.Get("msg_value",command)){
	      ret.Set("msg_value",Command(command, state));
	      ret.Set("AKN",true);
	    }
	    else{
	      *ret["msg_value"]="Error!!! No msg_value/Command in JSON";
	      ret.Set("AKN",false);
	      logger->Send("Warning!!! No slow control msg_value/Command in JSON");
	    }
	  }
	  else{
	    *ret["msg_value"]="Error!!! Unknown msg_type";
	    ret.Set("AKN",false);
	    logger->Send("Warning!!! Unknown slow control msg_type");
	  }
	  
	  //reply

	  long msg_id=0;
	  configuration_variables.Get("msg_id",msg_id);
	  
	  ret.Set("uuid",UUID);
	  ret.Set("msg_id",msg_id);
	  ret.Set("recv_time",boost::posix_time::to_simple_string(boost::posix_time::second_clock::universal_time()));    

	  std::vector<zmq::message_t*> reply;
	  reply.push_back(identity); 

	  zmq::message_t* blank= new zmq::message_t(0);
	  reply.push_back(blank);

	  reply.push_back(ret.MakeMsg());
 
	  msg_queue.push_back(reply);
	  
	}
	else logger->Send("Warning!!! Slow control recived unknown message format");  
	return false;
	
      }
      else logger->Send("Warning!!! Slow control recived bad message"); 
    }
    else logger->Send("Warning!!! Slow control recived bad message");             
  }
  else logger->Send("Warning!!! Slow control recived bad message");             

  return true;
}

std::string SlowControl::Command (std::string command, int &state){

  std::string ret="Warning!!! unknown Command received: "+command;

  if(command=="Start"){
    if(state==2) state=3;
    ret="Started";
  }
  else if(command=="Pause"){
    if(state==3) state=2;
    ret="Paused"; 
  }
  else if(command=="Stop"){
    state=0;
    ret="Stopping";
  }
  else if(command=="Init"){
    state=1;
    ret="Initialising";
  }
  else if(command=="Status"){
    std::stringstream tmp;
    tmp<<"Status is: "<<"Good"<< ", state is: "<<state;
    ret=tmp.str(); 
  }
  else if(command=="?"){
    if(state==1) ret="Avaiable commands are: Stop, Status, ?";
    else ret="Avaiable commands are: Start, Pause, Stop, Init, Status, ?";    
  }
  
  return ret;
  
}
 
bool SlowControl::Config (){

  configuration_variables.Print();

  return true;

}

bool SlowControl::Request(){

  std::vector<zmq::message_t*> reply1;
  std::vector<zmq::message_t*> reply2;

  std::string ident1="MPMTSC1";
  std::string ident2="MPMTSC2";

  zmq::message_t* identity1=new zmq::message_t(8);
  zmq::message_t* identity2=new zmq::message_t(8);
    
  snprintf ((char *) identity1->data(), ident1.length()+1 , "%s" , ident1.c_str());
  snprintf ((char *) identity2->data(), ident2.length()+1 , "%s" , ident2.c_str());

  reply1.push_back(identity1);
  reply2.push_back(identity2);       

  zmq::message_t* blank1= new zmq::message_t(0);
  zmq::message_t* blank2= new zmq::message_t(0);  

  reply1.push_back(blank1);
  reply2.push_back(blank2);


  Store tmp;
  tmp.Set("uuid",UUID);  
  *tmp["msg_type"]="Request";
  *tmp["msg_value"]="MPMT"; 
  tmp.Set("time",boost::posix_time::to_simple_string(boost::posix_time::second_clock::universal_time()));   


  reply1.push_back(tmp.MakeMsg());
  reply2.push_back(tmp.MakeMsg());        
  
  msg_queue.push_back(reply1);
  msg_queue.push_back(reply2);


  return true;

}
