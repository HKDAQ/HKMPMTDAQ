#include "Logger.h"

Logger::Logger():Tool(){}


bool Logger::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  if(!m_variables.Get("logger_port",logger_port)) logger_port="11111";

  logger_sock=new zmq::socket_t(*(m_data->context), ZMQ_SUB);
  logger_sock->setsockopt(ZMQ_SUBSCRIBE,"",0);

  utils= new Utilities(m_data->context);

  return true;
}


bool Logger::Execute(){

  utils->UpdateConnections("MPMT", logger_sock, connections, logger_port);

  zmq::message_t msg;

  if(logger_sock->recv(&msg, ZMQ_NOBLOCK)){

    std::istringstream iss(static_cast<char*>(msg.data()));

    Store tmp;
    tmp.JsonParser(iss.str());
    std::string from,uuid,time,message; 
    tmp.Get("From",from);
    tmp.Get("UUID",uuid);
    tmp.Get("Time",time);
    message=*tmp["msg"];

   
    *m_log<<blue<<"received log message:"<<std::endl;
    *m_log<<blue<<"From: "<<from<<","<<uuid<<std::endl;
    *m_log<<blue<<"Time: "<<time<<std::endl;                 
    *m_log<<blue<<"Msg: "<<message<<plain<<std::endl<<std::endl;              
    

  }

  return true;
}


bool Logger::Finalise(){

  delete logger_sock;
  logger_sock=0;

  delete utils;
  utils=0;

  connections.clear();

  return true;
}
