#include <Logger.h>

Logger::Logger(zmq::socket_t &insock, boost::posix_time::time_duration inperiod, std::string UUID, std::string service_name){

  sock=&insock;
  period=inperiod;
  last_message=boost::posix_time::second_clock::universal_time();
  JSONmsg.Set("From",service_name); 
  JSONmsg.Set("UUID",UUID);     

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
