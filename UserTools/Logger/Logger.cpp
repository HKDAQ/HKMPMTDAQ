#include "Logger.h"

Logger::Logger():Tool(){}


bool Logger::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  logger_sock=new zmq::socket_t(*(m_data->context), ZMQ_SUB);
  logger_sock->setsockopt(ZMQ_SUBSCRIBE,"",0);

  utils= new Utilities(m_data->context);

  return true;
}


bool Logger::Execute(){

  utils->UpdateConnections("MPMT", logger_sock, connections, "11111");

  zmq::message_t msg;

  if(logger_sock->recv(&msg, ZMQ_NOBLOCK)){

    std::istringstream iss(static_cast<char*>(msg.data()));
    
    std::cout<<blue<<"received log message:"<<std::endl;
    std::cout<<iss.str()<<plain<<std::endl;

  }

  return true;
}


bool Logger::Finalise(){

  return true;
}
