#ifndef MYTOOLZMQMultiThread_H
#define MYTOOLZMQMultiThread_H

#include <string>
#include <iostream>

#include "Tool.h"

struct MyToolZMQMultiThread_args:Thread_args{

  MyToolZMQMultiThread_args();
  ~MyToolZMQMultiThread_args();

};

/**
 * \class MyToolZMQMultiThread
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
 *
 * $Author: B.Richards $
 * $Date: 2019/05/28 10:44:00 $
 * Contact: b.richards@qmul.ac.uk
 */
class MyToolZMQMultiThread: public Tool {


 public:

  MyToolZMQMultiThread(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.


 private:

  static void Thread(Thread_args* arg);
  Utilities* m_util;
  std::vector<MyToolZMQMultiThread_args*> args;

  zmq::pollitem_t items[2]; 
  zmq::socket_t* ManagerSend;
  zmq::socket_t* ManagerReceive;

  int m_freethreads;

};


#endif
