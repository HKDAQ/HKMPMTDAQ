#ifndef SERVICEDISCOVERY_H
#define SERVICEDISCOVERY_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <Store.h>


class ServiceDiscovery{


public:

  ServiceDiscovery(std::string address, int port, std::string UUID, std::string service_name, int remote_port, int &return_sock);
void Send(std::string status);

Store message;

private:

struct sockaddr_in addr;      
int addrlen, sock, cnt;
struct ip_mreq mreq;

long msg_id;

};




#endif
