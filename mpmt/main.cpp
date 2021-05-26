#include <iostream>
#include <string>
#include <sstream>

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/uuid/uuid_generators.hpp> // generators  

#include <Logger.h>
#include <Store.h>
#include <ServiceDiscovery.h>
#include <SlowControl.h>
#include <Monitor.h>
#include <DataManager.h>

int main(int argc, char *argv[]){

  if(argc>2){
    std::cout<<"Incorrect number of arguments"<<std::endl;
    std::cout<<"useage: ./a.out <varaibles> "<<std::endl;
    exit(1);
  }

  /////////////variables and defaults//////////////////////

  std::string UUID=boost::uuids::to_string(boost::uuids::random_generator()());

  std::string logger_sock_port="tcp://*:11111";
  int logger_resend_period=1;

  int service_discovery_port=5000;
  std::string service_discovery_address="239.192.1.1";
  std::string service_name="MPMT";
  int broadcast_period=5;
  int service_discovery_sock=0;

  int slow_control_port=22222;
  std::stringstream tmp;
  tmp<<"tcp://*:"<<slow_control_port;
  std::string slow_control_sock_port=tmp.str();
  int slow_control_timeout=100;
  int config_request_period=1;

  std::string monitor_sock_port="tcp://*:33333";
  int monitor_send_period=10;

  std::string data_sock_port="tcp://*:4444";
  int data_timeout=100;
  int resend_period= 10;
  int resend_attempts=10;
  int queue_warning_limit=8000;
  int queue_max_size=10000;

  //////////////////////////////////////////////////////


  ////////////////////// Dynamic variable set and store defualts ///////////////////

  Store variables;
  if(argc==2) variables.Initialise(argv[1]);

  if(!variables.Get("UUID", UUID)) variables.Set("UUID", UUID);

  if(!variables.Get("logger_sock_port", logger_sock_port)) variables.Set("logger_sock_port", logger_sock_port);
  if(!variables.Get("logger_resend_period", logger_resend_period)) variables.Set("logger_resend_period", logger_resend_period);
  
  if(!variables.Get("service_discovery_port", service_discovery_port)) variables.Set("service_discovery_port", service_discovery_port);
  if(!variables.Get("service_discovery_address", service_discovery_address)) variables.Set("service_discovery_address", service_discovery_address);
  if(!variables.Get("service_name", service_name)) variables.Set("service_name", service_name);
  if(!variables.Get("broadcast_period", broadcast_period)) variables.Set("broadcast_period", broadcast_period);
  if(!variables.Get("service_discovery_sock", service_discovery_sock)) variables.Set("service_discovery_sock", service_discovery_sock);

  if(!variables.Get("slow_control_port",  slow_control_port)) variables.Set("slow_control_port", slow_control_port);
  if(!variables.Get("slow_control_sock_port", slow_control_sock_port)) variables.Set("slow_control_sock_port", slow_control_sock_port);
  if(!variables.Get("slow_control_timeout", slow_control_timeout)) variables.Set("slow_control_timeout", slow_control_timeout);
  if(!variables.Get("config_request_period", config_request_period)) variables.Set("config_request_period", config_request_period);
  
  if(!variables.Get("monitor_sock_port", monitor_sock_port)) variables.Set("monitor_sock_port", monitor_sock_port);
  if(!variables.Get("monitor_send_period", monitor_send_period)) variables.Set("monitor_send_period", monitor_send_period);

  if(!variables.Get("data_sock_port", data_sock_port)) variables.Set("data_sock_port", data_sock_port);
  if(!variables.Get("data_timeout", data_timeout)) variables.Set("data_timeout", data_timeout);
  if(!variables.Get("resend_period", resend_period)) variables.Set("resend_period", resend_period);
  if(!variables.Get("resend_attempts",  resend_attempts)) variables.Set("resend_attempts", resend_attempts);
  if(!variables.Get("queue_warning_limit", queue_warning_limit)) variables.Set("queue_warning_limit", queue_warning_limit);
  if(!variables.Get("queue_max_size",  queue_max_size)) variables.Set("queue_max_size", queue_max_size);


  //////////////////////////////////  


  //////////////////// Setting up sockets and classes ///////////////
  
  zmq::context_t context(1);
   
  zmq::socket_t logger_sock(context, ZMQ_PUB);
  Logger logger(logger_sock, variables);
   
  ServiceDiscovery service_discovery(variables, service_discovery_sock);
   
  zmq::socket_t slow_control_sock(context, ZMQ_ROUTER);
  SlowControl slow_control(slow_control_sock, logger, variables);
   
  zmq::socket_t monitor_sock(context, ZMQ_PUB);
  Monitor monitor(monitor_sock, variables);
   
  zmq::socket_t data_sock(context, ZMQ_DEALER);
  DataManager data_manager(data_sock, logger, variables);
   
  
  zmq::pollitem_t in_items[]={
    {slow_control_sock, 0 , ZMQ_POLLIN, 0},
    {data_sock, 0 , ZMQ_POLLIN, 0},
  };
   
  zmq::pollitem_t out_items[]={
    {slow_control_sock, 0 , ZMQ_POLLOUT, 0},
    {NULL, service_discovery_sock, ZMQ_POLLOUT, 0},
    {monitor_sock, 0 , ZMQ_POLLOUT, 0},
    {data_sock, 0 , ZMQ_POLLOUT, 0},
  };
  
  
  /////////////////////////////////////

  ////// timers ////
  
  boost::posix_time::time_duration broadcast_period_td= boost::posix_time::seconds(broadcast_period);
  boost::posix_time::ptime last_service_broadcast=boost::posix_time::second_clock::universal_time();

  boost::posix_time::time_duration config_request_period_td= boost::posix_time::seconds(config_request_period);
  boost::posix_time::ptime last_config_request=boost::posix_time::second_clock::universal_time();  

  boost::posix_time::time_duration monitor_send_period_td= boost::posix_time::seconds(monitor_send_period);
  boost::posix_time::ptime last_monitor_send=boost::posix_time::second_clock::universal_time();          

  //////////////////

  int state=1;


  while(state){

    //    sleep(3);

    if(state==2){
      data_manager.GetData(); 
      zmq::poll(&in_items[0], 2, 0);  
    }
    else{
      zmq::poll(&in_items[0], 2, 100); 
    }
    
   
    data_manager.ManageQueues();
    zmq::poll(&out_items[0], 4, 0);
   
    if(in_items[0].revents & ZMQ_POLLIN)  slow_control.Receive(state);
    
    
    boost::posix_time::time_duration lapse = config_request_period_td - (boost::posix_time::second_clock::universal_time() - last_config_request);
    
    if(lapse.is_negative() && state==1){
      
      slow_control.Request();    
      last_config_request=boost::posix_time::second_clock::universal_time();
    }
    
    
    if((out_items[0].revents & ZMQ_POLLOUT) && state!=0) slow_control.Send();


    
    lapse=broadcast_period_td - (boost::posix_time::second_clock::universal_time() - last_service_broadcast);
    
    if((out_items[1].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){

      service_discovery.Send(monitor.Status());
      last_service_broadcast=boost::posix_time::second_clock::universal_time();
    }


    lapse=monitor_send_period_td - (boost::posix_time::second_clock::universal_time() - last_monitor_send);


    if((out_items[2].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){

      monitor.Send(state);
      last_monitor_send=boost::posix_time::second_clock::universal_time();
    }
  

    if((in_items[1].revents & ZMQ_POLLIN) && state!=0) data_manager.Receive();

    if((out_items[3].revents & ZMQ_POLLOUT) && state!=0) data_manager.Send();

}
  
 
  return 0;
  
}
