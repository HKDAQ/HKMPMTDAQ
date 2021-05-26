#include <Monitor.h>

Monitor::Monitor(zmq::socket_t &insock, Store &variables){

  sock=&insock;

  std::string monitor_sock_port;

  variables.Get("monitor_sock_port",monitor_sock_port);

  sock->bind(monitor_sock_port.c_str());

}

bool Monitor::Send(int state){

  CollectData();

  monitoring_data.Set("time",boost::posix_time::to_simple_string(boost::posix_time::second_clock::universal_time()));
  monitoring_data.Set("state",state);    
  
  return monitoring_data.Send(sock);


}


void Monitor::CollectData(){

  monitoring_data.Set("status","Good");    

}

std::string Monitor::Status(){

  return "Good";

}
