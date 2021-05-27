#include <iostream>
#include <string>
#include <sstream>

#include <zmq.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>            
#include <boost/uuid/uuid_io.hpp>         
#include <boost/uuid/uuid_generators.hpp> 

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

  ///////////// Variables and Defaults //////////////////////

  Store variables; // simple dynamic omni storage class

  std::string UUID=boost::uuids::to_string(boost::uuids::random_generator()());  // unique identifier
  
  int broadcast_period=5;       // seconds between service discovery broadcasts
  int config_request_period=1;  // number of seconds between requests for initial configurat
  int monitor_send_period=10;   // number of seconds between monitoring information is given out
  int service_discovery_sock=0;  // endpoint for service discovery socket
  
  variables.Set("UUID",UUID);
  variables.Set("broadcast_period",broadcast_period);
  variables.Set("config_request_period",config_request_period);
  variables.Set("monitor_send_period",monitor_send_period);
  
  if(argc==2){
    variables.Initialise(argv[1]); // loading dynamic variables file
    
    variables.Get("UUID",UUID);    // loading variables dynamically from variables file
    variables.Get("broadcast_period",broadcast_period); 
    variables.Get("config_request_period",config_request_period);
    variables.Get("monitor_send_period",monitor_send_period);
  
  }
  //////////////////////////////////////////////////////



  //////////////////// Setting up sockets and classes ///////////////
  
  zmq::context_t context(1);     // ZMQ context for managing socket queues
   
  zmq::socket_t logger_sock(context, ZMQ_PUB);  // socket for sening out logging/warning messages
  Logger logger(logger_sock, variables);        // class for handeling formating and sending of logging messages
   
  ServiceDiscovery service_discovery(variables, service_discovery_sock);  // class for handelling service discovery broadcasts
   
  zmq::socket_t slow_control_sock(context, ZMQ_ROUTER);  // socket for slow control input and output
  SlowControl slow_control(slow_control_sock, logger, variables); // class for handelling slow control input, commands and output
   
  zmq::socket_t monitor_sock(context, ZMQ_PUB);       // socket for sending monitoring information out
  Monitor monitor(monitor_sock, variables);       // class to handel collecting and sending monitoring data
   
  zmq::socket_t data_sock(context, ZMQ_DEALER);    // socket for sending out data and receiving acknolwedge statements
  DataManager data_manager(data_sock, logger, variables);  // class for handeling data queues and collecting data
   
  
  zmq::pollitem_t in_items[]={               // input sockets to poll (similar to select) for activity 
    {slow_control_sock, 0 , ZMQ_POLLIN, 0},
    {data_sock, 0 , ZMQ_POLLIN, 0},  
  };
    
  zmq::pollitem_t out_items[]={              // output sockets to poll (similar to select) for activity
    {slow_control_sock, 0 , ZMQ_POLLOUT, 0},
    {NULL, service_discovery_sock, ZMQ_POLLOUT, 0},
    {monitor_sock, 0 , ZMQ_POLLOUT, 0},
    {data_sock, 0 , ZMQ_POLLOUT, 0},
  };
  
  /////////////////////////////////////



  ////// timers ////
  
  boost::posix_time::time_duration broadcast_period_td= boost::posix_time::seconds(broadcast_period); // converting service discovery period variable to boost time_duration
  boost::posix_time::ptime last_service_broadcast=boost::posix_time::second_clock::universal_time();  // variable for storing time of last service discovery broadcast

  boost::posix_time::time_duration config_request_period_td= boost::posix_time::seconds(config_request_period);  // converting slow control configuration request period variable to boost time_duration    
  boost::posix_time::ptime last_config_request=boost::posix_time::second_clock::universal_time();   // variable for storing time of last configuration request

  boost::posix_time::time_duration monitor_send_period_td= boost::posix_time::seconds(monitor_send_period); // converting monitoring period variable to boost time_duration
  boost::posix_time::ptime last_monitor_send=boost::posix_time::second_clock::universal_time();     // variable for storing time of last monitoring braodcast

  //////////////////

  int state=1;       //variables to define the state of the system 0=Shutdown, 1=Initialisation, 2=Paused not taking data but initialised, 3=Taking data

  variables.Set("state",state);          // loading state variable into varaibles Store so all classes can access it
  variables.Set("status","Booting");       // loading current system status into variables Store so all classes can access it 

  while(state){       // main execution loop while state is anything other than shutdown

    if(state==3){               // if in data taking state
      data_manager.GetData();        // Get data from hardware (I would normally thread this so it can run asynchronously, but trying limit numbers of threads and the rest of the code is pretty light weight so up to you. Currently i have just gnerated some fake data in it).
      zmq::poll(&in_items[0], 2, 0);       // there is no blocking on the polling here to not limit/holdup the collection of data 
    }
    else{
      zmq::poll(&in_items[0], 2, 100);       // in any other state other than data taking i have a 100ms block on the poll to keep the CPU from maxing out doing nothing and power consupmtion low.
    }
    
   
    data_manager.ManageQueues();         // this function manages the two queues of data the new data queue that needs to be sent out and the sent queue that is data waiting to be deleted when an acknolwedgement is received. It also handells sending out warnings and deleteing old data if the queues get too large and moving data between the queues for resending.
    zmq::poll(&out_items[0], 4, 0);       // polling the outbound sockets. This poll ahs no block as can be delayed by the inboound socket polling. 
   
    if(in_items[0].revents & ZMQ_POLLIN) slow_control.Receive();       // reading in any data received on the slowcontrol port and taking the necessary actions
    
    boost::posix_time::time_duration lapse = config_request_period_td - (boost::posix_time::second_clock::universal_time() - last_config_request);       //determining if the the slow control configuration request timer has lapsed
    
    if(lapse.is_negative() && state==1){       // only want to send out these requests in the intialising state 
      
      slow_control.Request();       // sending out the request for configuration
      last_config_request=boost::posix_time::second_clock::universal_time();       //resetting last time request sent;
    
    }
    
    
    if((out_items[0].revents & ZMQ_POLLOUT) && state!=0) slow_control.Send();              // sending any data necesary on the slow control socket
    
    
    lapse=broadcast_period_td - (boost::posix_time::second_clock::universal_time() - last_service_broadcast);          //checking if service discovery time has lapsed
    
    if((out_items[1].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){
      
      service_discovery.Send();         //sending out service discovery braodcast
      last_service_broadcast=boost::posix_time::second_clock::universal_time();          // reseeting last time
    
    }


    lapse=monitor_send_period_td - (boost::posix_time::second_clock::universal_time() - last_monitor_send);       // checing if monitoring time has lapsed


    if((out_items[2].revents & ZMQ_POLLOUT) && lapse.is_negative() && state!=0){
     
      data_manager.UpdateStatus();   // updating the queue status into the variables Store
      monitor.Send();      /// sedning out monitoring info
      last_monitor_send=boost::posix_time::second_clock::universal_time();  // resetting time of last send
     
    }
  

    if((in_items[1].revents & ZMQ_POLLIN) && state!=0)  data_manager.Receive();  //receiving any data acknolwedge statements
    
    if((out_items[3].revents & ZMQ_POLLOUT) && state!=0) data_manager.Send();  // sending out any data

    variables.Get("state",state); // Get the current state from store incase classes have changed it           
    
  }  
  
  return 0;
  
}
