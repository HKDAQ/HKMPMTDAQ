#include "DataReceiver.h"

DataReceiver::DataReceiver():Tool(){}


bool DataReceiver::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("data_port",data_port)) data_port="44444";

  data_sock=new zmq::socket_t(*(m_data->context), ZMQ_ROUTER);

  utils= new Utilities(m_data->context);

  items[0].socket=*data_sock;
  items[0].fd=0;
  items[0].events=ZMQ_POLLIN;
  items[0].revents=0;

  period=boost::posix_time::seconds(1);

  sum=0;
  last=boost::posix_time::second_clock::universal_time();

  return true;
}


bool DataReceiver::Execute(){

  utils->UpdateConnections("MPMT", data_sock, connections, data_port);


  zmq::poll(&items[0], 1, 100);

  if(items[0].revents & ZMQ_POLLIN){
    
    
    zmq::message_t identity;
    data_sock->recv(&identity); 

    MPMTDataChunk tmp;
  

    if(tmp.Receive(data_sock)){


      sum+=tmp.hits.size();
      
      Store reply;
      reply.Set("data_id", tmp.data_id);
 
      std::string msg_string;
      reply>>msg_string;
     
      zmq::message_t msg(msg_string.size()+1);


      snprintf ((char *) msg.data(), msg_string.length()+1 , "%s" , msg_string.c_str());
      data_sock->send(identity, ZMQ_SNDMORE);
      data_sock->send(msg);
    }
    else *m_log<<red<<"receive data problem"<<std::endl;



  }

  boost::posix_time::time_duration lapse = period -( boost::posix_time::second_clock::universal_time() - last);


  if(lapse.is_negative() && sum>0){

    *m_log<<"Received: "<<sum<<" hits in the last "<<period<<std::endl;

    last= boost::posix_time::second_clock::universal_time();
    sum=0;
  }


	
  return true;
}


bool DataReceiver::Finalise(){

  delete data_sock;
  data_sock=0;

  delete utils;
  utils=0;

  connections.clear();

  return true;
}
