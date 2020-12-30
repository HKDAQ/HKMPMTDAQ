#include "DummyTool.h"

DummyTool::DummyTool():Tool(){}


bool DummyTool::Initialise(std::string configfile, DataModel &data){

   if(configfile!="")  m_variables.Initialise(configfile);
   //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log; 

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;  
 
  m_verbose=6;
  Log("test 1",1,m_verbose);

  return true;
}


bool DummyTool::Execute(){

  Log("test 2",2,m_verbose);
  Log("test 2",5,5);
  int a=2;
  *m_log<<red<<"hello"<<green<<a<<"hi"<<std::endl<<"hello"<<a<<a<<"oooyeh"<<std::endl;
  *m_log<<MsgL(2,6)<<yellow<<"hello"<<green<<a<<"hi"<<std::endl<<"hello"<<a<<a<<"oooyeh"<<std::endl;
  *m_log<<ML(2)<<pink<<"hello"<<green<<a<<"hi"<<std::endl<<"hello"<<a<<a<<"oooyeh"<<std::endl;
  *m_log<<cyan<<"hello"<<green<<a<<"hi"<<std::endl<<"hello"<<a<<a<<"oooyeh"<<std::endl;
  *m_log<<MsgL(1,1);
  MLC();
  return true;
}


bool DummyTool::Finalise(){

  return true;
}
