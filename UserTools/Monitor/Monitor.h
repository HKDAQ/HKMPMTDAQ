#ifndef Monitor_H
#define Monitor_H

#include <string>
#include <iostream>

#include "Tool.h"


/**
 * \class Monitor
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class Monitor: public Tool {


 public:

  Monitor(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.


 private:

  zmq::socket_t* monitor_sock;
  DAQUtilities* utils;
  std::map<std::string,Store*> connections;
  std::string monitor_port;


};


#endif
