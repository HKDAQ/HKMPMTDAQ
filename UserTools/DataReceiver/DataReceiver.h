#ifndef DataReceiver_H
#define DataReceiver_H

#include <string>
#include <iostream>

#include "Tool.h"


/**
 * \class DataReceiver
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class DataReceiver: public Tool {


 public:

  DataReceiver(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.


 private:

  zmq::socket_t* data_sock; 
  DAQUtilities* utils;
  std::map<std::string,Store*> connections;
  zmq::pollitem_t items[1];
  unsigned long hitsum;
  unsigned long msgsum;
  boost::posix_time::time_duration period;
  boost::posix_time::ptime last;
  std::string data_port;

};


#endif
