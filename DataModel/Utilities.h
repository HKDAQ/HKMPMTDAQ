#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <zmq.hpp>
#include <sstream>
#include <pthread.h>
#include <map>
#include <Store.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

struct Thread_args{

  Thread_args(){ 
    kill=false;
  }

  Thread_args(zmq::context_t* contextin, std::string threadname,  void (*funcin)(std::string)){
   
    context=contextin;
    ThreadName=threadname;
    func_with_string=funcin;
    kill=false; 
  }

  Thread_args(zmq::context_t* contextin, std::string threadname,  void (*funcin)(Thread_args*)){
    
    context=contextin;
    ThreadName=threadname;
    func=funcin;
    kill=false;
  }      

  virtual ~Thread_args(){
    running =false;
    kill=true;
    delete sock;
    sock=0;
  }

  zmq::context_t *context;
  std::string ThreadName;
  void (*func_with_string)(std::string);
  void (*func)(Thread_args*);
  pthread_t thread;
  zmq::socket_t* sock;
  bool running;
  bool kill;
  
};

class Utilities{
  
 public:
  
  Utilities(zmq::context_t* zmqcontext);
  bool AddService(std::string ServiceName, unsigned int port, bool StatusQuery=false);
  bool RemoveService(std::string ServiceName);
  int UpdateConnections(std::string ServiceName, zmq::socket_t* sock, std::map<std::string,Store*> &connections);
  Thread_args* CreateThread(std::string ThreadName,  void (*func)(std::string));  //func = &my_int_func;
  Thread_args* CreateThread(std::string ThreadName,  void (*func)(Thread_args*), Thread_args* args);
  bool MessageThread(Thread_args* args, std::string Message, bool block=true);
  bool MessageThread(std::string ThreadName, std::string Message, bool block=true);
  bool KillThread(Thread_args* &args); 
  bool KillThread(std::string ThreadName);

  template <typename T>  bool SendPointer(zmq::socket_t* sock, T* pointer){
    
    std::stringstream tmp;
    tmp<<pointer;

    zmq::message_t message(tmp.str().length()+1);
    snprintf ((char *) message.data(), tmp.str().length()+1 , "%s" , tmp.str().c_str()) ;

   return sock->send(message);  

  }
  
  template<typename T> bool ReceivePointer(zmq::socket_t* sock, T*& pointer){  
   
    zmq::message_t message;
    
    if(sock->recv(&message)){   
      
      std::istringstream iss(static_cast<char*>(message.data()));
      
      //      long long unsigned int tmpP;
      unsigned long tmpP;
      iss>>std::hex>>tmpP;
      
      pointer=reinterpret_cast<T*>(tmpP);
      
      return true;
    }
    
    else { 
      pointer=0;
      return false;
    }
    
  }

     
    
    
  private:
    
    zmq::context_t *context;
    static void* String_Thread(void *arg);
    static void* Thread(void *arg);
    std::map<std::string, Thread_args*> Threads;
    
        
  };
  
  
#endif 
