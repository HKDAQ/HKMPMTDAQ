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

int main(int argc, char *argv[]){

  /////////////variables //////////////////////

  std::string UUID=boost::uuids::to_string(boost::uuids::random_generator()());

  std::string logger_sock_port="tcp://*:11111";
  boost::posix_time::time_duration logger_resend_period= boost::posix_time::seconds(1);

  int service_discovery_port=5000;
  std::string service_discovery_address="239.192.1.1";
  std::string service_name="MPMT";
  boost::posix_time::time_duration broadcast_period= boost::posix_time::seconds(5);
  int service_discovery_sock=0;
  
  int slow_control_port=22222;
  std::stringstream tmp;
  tmp<<"tcp://*:"<<slow_control_port;
  std::string slow_control_sock_port=tmp.str();
  int slow_control_timeout=100;
  boost::posix_time::time_duration config_request_period= boost::posix_time::seconds(1);

  std::string monitor_sock_port="tcp://*:33333";
  boost::posix_time::time_duration monitor_send_period= boost::posix_time::seconds(10);

  //////////////////////////////////  


  //////////////////// SETTING UP SOCKS ///////////////

  zmq::context_t context(1);

  zmq::socket_t logger_sock(context, ZMQ_PUB);
  logger_sock.bind(logger_sock_port.c_str());
  Logger logger(logger_sock,logger_resend_period, UUID, service_name);

  ServiceDiscovery service_discovery(service_discovery_address, service_discovery_port, UUID, service_name, slow_control_port, service_discovery_sock);

  zmq::socket_t slow_control_sock(context, ZMQ_ROUTER);
  slow_control_sock.bind(slow_control_sock_port.c_str());
  slow_control_sock.setsockopt(ZMQ_RCVTIMEO, slow_control_timeout);
  slow_control_sock.setsockopt(ZMQ_SNDTIMEO, slow_control_timeout);
  SlowControl slow_control(slow_control_sock, UUID, logger);


  zmq::socket_t monitor_sock(context, ZMQ_PUB);
  monitor_sock.bind(monitor_sock_port.c_str());
  Monitor monitor(monitor_sock);

  zmq::pollitem_t in_items[]={
    {slow_control_sock, 0 , ZMQ_POLLIN, 0},
  };

  zmq::pollitem_t out_items[]={
    {slow_control_sock, 0 , ZMQ_POLLOUT, 0},
    {NULL, service_discovery_sock, ZMQ_POLLOUT, 0},
    {monitor_sock, 0 , ZMQ_POLLOUT, 0},
  };
  

  /////////////////////////////////////

  ////// timers ////

  boost::posix_time::ptime last_service_broadcast=boost::posix_time::second_clock::universal_time();
  boost::posix_time::ptime last_config_request=boost::posix_time::second_clock::universal_time();  
  boost::posix_time::ptime last_monitor_send=boost::posix_time::second_clock::universal_time();          

  //////////////////

  int state=1;


  while(state){


    //    sleep(3);

    if(state!=2) zmq::poll(&in_items[0], 1, 100);  
    else zmq::poll(&in_items[0], 1, 0); 
    
    if(in_items[0].revents & ZMQ_POLLIN)  slow_control.Receive(state);
    
    zmq::poll(&out_items[0], 3, 0);
        
    boost::posix_time::time_duration lapse = config_request_period - (boost::posix_time::second_clock::universal_time() - last_config_request);
    
    if(lapse.is_negative() && state==1){
      
      slow_control.Request();    
      last_config_request=boost::posix_time::second_clock::universal_time();
    }
    
    
    if((out_items[0].revents & ZMQ_POLLOUT) && state!=0) slow_control.Send();


    
    lapse=broadcast_period - (boost::posix_time::second_clock::universal_time() - last_service_broadcast);
    
    if((out_items[1].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){

      service_discovery.Send("Good");
      last_service_broadcast=boost::posix_time::second_clock::universal_time();
    }


    lapse=monitor_send_period - (boost::posix_time::second_clock::universal_time() - last_monitor_send); 


    if((out_items[2].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){

      monitor.Send(state);
      last_monitor_send=boost::posix_time::second_clock::universal_time();
    }
  

}
  
 
  return 0;
  
}
