#include "MyToolZMQMultiThread.h"

MyToolZMQMultiThread_args::MyToolZMQMultiThread_args():Thread_args(){}

MyToolZMQMultiThread_args::~MyToolZMQMultiThread_args(){}


MyToolZMQMultiThread::MyToolZMQMultiThread():Tool(){}


bool MyToolZMQMultiThread::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;

  int threadcount=0;
  if(!m_variables.Get("Threads",threadcount)) threadcount=4;

  m_util=new Utilities(m_data->context);

  ManagerSend=new zmq::socket_t(*m_data->context,ZMQ_PUSH);
  ManagerSend->bind("inproc://MyToolZMQMultiThreadSend");
  ManagerReceive=new zmq::socket_t(*m_data->context,ZMQ_PULL);
  ManagerReceive->bind("inproc://MyToolZMQMultiThreadReceive");

  items[0].socket=*ManagerSend;
  items[0].fd=0;
  items[0].events=ZMQ_POLLOUT;
  items[0].revents=0;
  items[1].socket=*ManagerReceive;
  items[1].fd=0;
  items[1].events=ZMQ_POLLIN;
  items[1].revents=0;
  
  for(int i=0;i<threadcount;i++){
    MyToolZMQMultiThread_args* tmparg=new MyToolZMQMultiThread_args();   
    args.push_back(tmparg);
    std::stringstream tmp;
    tmp<<"T"<<i; 
    m_util->CreateThread(tmp.str(), &Thread, args.at(i));
  }
  
  m_freethreads=threadcount;
  
    
  
  return true;
}


bool MyToolZMQMultiThread::Execute(){

  zmq::poll(&items[0], 2, 0);

  if ((items[1].revents & ZMQ_POLLIN)){

    zmq::message_t message;
    ManagerReceive->recv(&message);
    std::istringstream iss(static_cast<char*>(message.data()));
    std::cout<<"reply = "<<iss.str()<<std::endl;
    m_freethreads++;

  }

  if ((items[0].revents & ZMQ_POLLOUT)){

    if(m_freethreads>0){

      //     Utilities::SendPointer(ManagerSend,pointer);
      std::string greeting="HI";
      zmq::message_t message(greeting.length()+1);
      snprintf ((char *) message.data(), greeting.length()+1 , "%s" , greeting.c_str()) ;
      ManagerSend->send(message);
      m_freethreads--;
      std::cout<<"sending Hi"<<std::endl;
    }

  }

  std::cout<<"free threads="<<m_freethreads<<":"<<args.size()<<std::endl;
  sleep(1);
  return true;
}


bool MyToolZMQMultiThread::Finalise(){

  for(int i=0;i<args.size();i++)  m_util->KillThread(args.at(i));
    
  args.clear();
  
  delete m_util;
  m_util=0;

  return true;
}

void MyToolZMQMultiThread::Thread(Thread_args* arg){

  MyToolZMQMultiThread_args* args=reinterpret_cast<MyToolZMQMultiThread_args*>(arg);

  zmq::socket_t ThreadReceive(*args->context,ZMQ_PULL);
  ThreadReceive.connect("inproc://MyToolZMQMultiThreadSend");
  zmq::socket_t ThreadSend(*args->context,ZMQ_PUSH);
  ThreadSend.connect("inproc://MyToolZMQMultiThreadReceive");
  
  zmq::pollitem_t initems[1] = {{ThreadReceive,0,ZMQ_POLLIN,0}};
  zmq::pollitem_t outitems[1] = {{ThreadSend,0,ZMQ_POLLOUT,0}};
  
  zmq::poll(&initems[0], 1, 100);

  if ((initems[0].revents & ZMQ_POLLIN)){
  
    zmq::message_t message;
    ThreadReceive.recv(&message);
    std::istringstream iss(static_cast<char*>(message.data()));
  
    sleep(10);

    zmq::poll(&outitems[0], 1, 10000);
    if ((outitems[0].revents & ZMQ_POLLOUT)){
      
      std::string greeting="hello";
      zmq::message_t message(greeting.length()+1);
      snprintf ((char *) message.data(), greeting.length()+1 , "%s" , greeting.c_str()) ;
      ThreadSend.send(message);
    }
    
  }

}
