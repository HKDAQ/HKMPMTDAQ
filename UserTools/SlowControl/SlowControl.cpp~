#include "SlowControl.h"

SlowControl::SlowControl():Tool(){}


bool SlowControl::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("slow_control_port", slow_control_port)) slow_control_port="22222";

  slow_control_sock=new zmq::socket_t(*(m_data->context), ZMQ_ROUTER);
  slow_control_sock->setsockopt(ZMQ_IDENTITY,"MPMTSC1",8);

  utils= new Utilities(m_data->context);

  items[0].socket=*slow_control_sock;
  items[0].fd=0;
  items[0].events=ZMQ_POLLIN;
  items[0].revents=0;

  return true;
}


bool SlowControl::Execute(){

  utils->UpdateConnections("MPMT", slow_control_sock, connections, slow_control_port);

  zmq::poll(&items[0], 1, 0);
 
  if (items[0].revents & ZMQ_POLLIN){
     
    zmq::message_t identity;
    if(slow_control_sock->recv(&identity)){

      zmq::message_t blank;
      if(slow_control_sock->recv(&blank)){

	zmq::message_t msg;
	if(slow_control_sock->recv(&msg)){
      
	  std::istringstream iss(static_cast<char*>(msg.data()));
	  
	  *m_log<<orange<<"received slowcontrol message:"<<std::endl;
	  
	  *m_log<<orange<<iss.str()<<plain<<std::endl<<std::endl;

	  Store tmp; 
	  tmp.JsonParser(iss.str());

	  std::string type="";
	  tmp.Get("msg_type",type);

	  if(type=="Request"){
	    
	    slow_control_sock->send(identity, ZMQ_SNDMORE);
	    slow_control_sock->send(blank, ZMQ_SNDMORE);
	    
	    Store reply;
	    reply.Set("msg_type","Config");
	    reply.Set("voltage1","333");	  
	    reply.Set("msg_id","0");
	    
	    std::string msg_str="";
	    reply>>msg_str;
	  
	    *m_log<<yellow<<"Replied with:"<<std::endl;
	    *m_log<<yellow<<msg_str<<plain<<std::endl<<std::endl;
  
	    zmq::message_t conifg_msg(msg_str.length()+1);
	    
	    snprintf ((char *) conifg_msg.data(), msg_str.length()+1 , "%s" , msg_str.c_str());
	    
	    slow_control_sock->send(conifg_msg);
	  }
	}
      }
    }
  }
  
    
  return true;
}


bool SlowControl::Finalise(){

  delete slow_control_sock;
  slow_control_sock=0;

  delete utils;
  utils=0;

  connections.clear();

  return true;
}
