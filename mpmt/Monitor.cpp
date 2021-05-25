#include <Monitor.h>

Monitor::Monitor(zmq::socket_t &insock){

  sock=&insock;

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
