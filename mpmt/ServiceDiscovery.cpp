#include <ServiceDiscovery.h>


ServiceDiscovery::ServiceDiscovery(std::string address, int port, std::string UUID, std::string service_name, int remote_port, int &return_sock){

  
  // set up socket //                                       
  
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct linger l;
  l.l_onoff  = 0;
  l.l_linger = 0;
  setsockopt(sock, SOL_SOCKET, SO_LINGER,(char *) &l, sizeof(l));
  
  //fcntl(sock, F_SETFL, O_NONBLOCK);     
  
  if (sock < 0) {
    perror("socket");
    printf("Failed to connect to multicast publish socket");
    exit(1);
  }
  bzero((char *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  addrlen = sizeof(addr);
  
  // send //               
  
  addr.sin_addr.s_addr = inet_addr(address.c_str());
  
  int m_remoteport=00000;

  message.Set("uuid",UUID);   
  message.Set("remote_port",remote_port);
  message.Set("status_query",false);
  message.Set("msg_type","Service Discovery");  
  message.Set("msg_value",service_name);

  msg_id=0;
  
  return_sock=sock;
}

void ServiceDiscovery::Send(std::string status){
  
  msg_id++;

  message.Set("status",status);
  message.Set("msg_id",msg_id);   

  boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
  
  std::stringstream isot;
  isot<<boost::posix_time::to_iso_extended_string(t) << "Z";
  
  message.Set("msg_time",isot.str());
  
  std::string pubmessage;
  message>>pubmessage;
  
  char msg[pubmessage.length()+1];
  
  snprintf(msg, pubmessage.length()+1 , "%s" , pubmessage.c_str() ) ;
  
  cnt = sendto(sock, msg , sizeof(msg), 0,(struct sockaddr *) &addr, addrlen);
      


}
