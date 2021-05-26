#include <Logger.h>

Logger::Logger(zmq::socket_t &insock, Store &variables){

  sock=&insock;

  std::string logger_sock_port;
  int logger_resend_period=0;
  std::string service_name;
  std::string UUID;

  variables.Get("logger_resend_period",logger_resend_period);
  variables.Get("service_name",service_name);
  variables.Get("UUID",UUID);
  variables.Get("logger_sock_port",logger_sock_port);  

  sock->bind(logger_sock_port.c_str());

  JSONmsg.Set("From",service_name);
  JSONmsg.Set("UUID",UUID);

  period= boost::posix_time::seconds(logger_resend_period);

  last_message=boost::posix_time::second_clock::universal_time();   

}

void Logger::Send(std::string message){

  boost::posix_time::time_duration lapse= period-(boost::posix_time::second_clock::universal_time()-last_message);
  
  if(lapse.is_negative()) messages.clear();
  
  for(std::vector<std::string>::iterator it=messages.begin(); it!=messages.end(); it++){
    
    if(message==(*it)) return;
    
  }
  
  messages.push_back(message);
  
  JSONmsg.Set("msg",message);
  
  JSONmsg.Set("Time",boost::posix_time::to_simple_string(boost::posix_time::second_clock::universal_time()));
  
  JSONmsg.Send(sock);
  
  last_message=boost::posix_time::second_clock::universal_time();
  
}
