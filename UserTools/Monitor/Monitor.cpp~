#include "Monitor.h"

Monitor::Monitor():Tool(){}


bool Monitor::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  if(!m_variables.Get("monitor_port",monitor_port)) monitor_port="33333";

  monitor_sock=new zmq::socket_t(*(m_data->context), ZMQ_SUB);
  monitor_sock->setsockopt(ZMQ_SUBSCRIBE,"",0);

  utils= new Utilities(m_data->context);

  return true;
}


bool Monitor::Execute(){

  utils->UpdateConnections("MPMT", monitor_sock, connections, monitor_port);

  zmq::message_t msg;

  if(monitor_sock->recv(&msg, ZMQ_NOBLOCK)){

    std::istringstream iss(static_cast<char*>(msg.data()));
    
    std::cout<<green<<"received monitoring message:"<<std::endl;
    
    Store tmp;
    tmp.JsonParser(iss.str());
    tmp.Print();
    
     *m_log<<plain<<std::endl;
     
  }

  return true;
}


bool Monitor::Finalise(){


  delete monitor_sock;
  monitor_sock=0;

  delete utils;
  utils=0;

  connections.clear();

  return true;
}
