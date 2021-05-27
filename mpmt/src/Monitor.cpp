#include <Monitor.h>

Monitor::Monitor(zmq::socket_t &insock, Store &invariables){

  sock=&insock;
  variables=&invariables;

  std::string monitor_sock_port="tcp://*:33333";  //  endpoint and port to bind monitoring socket to

  variables->Get("monitor_sock_port",monitor_sock_port);

  sock->bind(monitor_sock_port.c_str());

}

bool Monitor::Send(){

  CollectData();

  monitoring_data.Set("time",boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::universal_time()));
   
  return monitoring_data.Send(sock);


}


void Monitor::CollectData(){

  //collect all your monitoring data here and fill the monitoring data Store

  float temp1=28.5;
  float temp2=43.7;
  int voltage1=1300;
  int voltage2=1280;
  //......
  //......
  std::stringstream status;
  int state=0;
  int data_queue=0;
  int sent_queue=0;
  int max_queue=0;

  variables->Get("data_queue", data_queue);
  variables->Get("sent_queue", sent_queue);
  variables->Get("max_queue", max_queue);
  variables->Get("state",state);

  status<<"State="<<state<<"/";
  if(state==1) status<<"Initialising";
  if(state==2) status<<"Paused";
  if(state==3) status<<"Taking_Data";
  status<<",Data:Sent:Total/Capacity="<<data_queue<<":"<<sent_queue<<":"<<data_queue+sent_queue<<"/"<<max_queue;
  variables->Set("status",status.str());

  monitoring_data.Set("status",status.str());
  monitoring_data.Set("state",state);
  monitoring_data.Set("data_queue", data_queue);
  monitoring_data.Set("sent_queue", sent_queue);
  monitoring_data.Set("max_queue", max_queue);
  monitoring_data.Set("temp1",temp1);    
  monitoring_data.Set("temp2",temp2);
  monitoring_data.Set("voltage1",voltage1);
  monitoring_data.Set("voltage2",voltage2);

}

