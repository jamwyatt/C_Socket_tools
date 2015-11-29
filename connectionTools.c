#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
extern int h_errno;

#include <errno.h>
extern int errno;

#include "error.h"
#include "connectionTools.h"

int messageTrace=0;
void setMessageTrace(int i) {
	messageTrace=i;
	}


int connectToServer(const char *hostname, int cport, int timeOut) {

	struct sockaddr_in sa;
	int val;
	int sock;
	struct hostent *host;

	if(cport == 0) {
		error("connectToServer bad port number");
		return(-1);
		}

	// Resolve the name to ip address ...
	if(!(host = gethostbyname(hostname))) {
		error("gethostbyname unknown host");
		return(-1);
		}

	char addr[1024];
	if(host->h_addr_list)
		inet_ntop(AF_INET, *host->h_addr_list,addr,sizeof(addr));
	else {
		error("Unable to resolve hostname to an address");
		return(-1);
		}

	// Connect to the address
	memset(&sa,'\0',sizeof(sa));
	if ((sock=socket(PF_INET,SOCK_STREAM,0)) < 0) {
		error("socket");
		return(-1);
		}

	sa.sin_family = PF_INET;
	sa.sin_port = htons(cport);
	if (inet_pton(PF_INET,addr, &sa.sin_addr.s_addr)<0) {
		error("inet_pton");
		return(-1);
		}

	char errorStr[1024];
        int flags;
        flags= fcntl(sock,F_GETFL,0);
        fcntl(sock,F_SETFL,flags | O_NONBLOCK); // Non-blocking socket

	// Connect - should return immediately
	if (connect(sock,(struct sockaddr *)&sa,sizeof(sa)) < 0) {
		if(errno != EINPROGRESS) {
			snprintf(errorStr, sizeof(errorStr),"connect[%s]",hostname);
			error(errorStr);
			return(-1);
			}
		errno=0;
		}

	fd_set readSet;
	fd_set writeSet;
	FD_ZERO(&readSet);
	FD_SET(sock,&readSet);
	writeSet = readSet;
	struct timeval tv;
	tv.tv_sec = timeOut;
	tv.tv_usec= 0;
	// Wait for the connect response or timeout
	if(select(sock+1,&readSet,&writeSet,NULL,&tv) == 0) {
		// Timeout
		errno = ETIMEDOUT;
		snprintf(errorStr, sizeof(errorStr),"connect[%s:%d]",hostname,cport);
		error(errorStr);
		return(-1);
		}

	int errorVal;
	socklen_t len=sizeof(errorVal);
	// Should be readable or writeable for the connect
	if(FD_ISSET(sock,&readSet) || FD_ISSET(sock,&writeSet)) {
		// Check for errors
		if(getsockopt(sock,SOL_SOCKET, SO_ERROR, &errorVal, &len) < 0) {
			close(sock);
			snprintf(errorStr, sizeof(errorStr),"connect/getsockopt[%s]",hostname);
			error(errorStr);
			return(-1);
			}
		if(errorVal) {
			// Something went bad!
			close(sock);
			errno = errorVal;
			snprintf(errorStr, sizeof(errorStr),"connect/getsockopt[%s]",hostname);
			error(errorStr);
			return(-1);
			}
		}
	else {
		// Can't be a timeout and the socket is readble/writeable!
		close(sock);
		snprintf(errorStr, sizeof(errorStr),"connect(unlikely)[%s]",hostname);
		error(errorStr);
		return(-1);
		}

	fcntl(sock,F_SETFL,flags);	// Back to blocking
	errno=0;
	if(messageTrace)
		fprintf(stderr,"Connected Socket=%d\n",sock);
	return(sock);
	}



// Read bytes from the socket until the timeout, or the socket closes
// or the buffer is filled, or a newline is found (and consumed)
int readLine(int sock, char *buff, int buffSize, int timeoutSeconds) {
	int retVal;
	struct timeval tv;
	fd_set readSet;

	FD_ZERO(&readSet);
	FD_SET(sock,&readSet);
	tv.tv_sec = 0;
	tv.tv_usec = 500;

	int bytesRead = 0;
	time_t start = time(NULL);
	while((bytesRead < (buffSize-1)) && (time(NULL) - start < timeoutSeconds)
				&&(((retVal = select(sock+1,&readSet,NULL,NULL,&tv)) != -1))||(errno==EINTR)) {

		if(retVal > 0) {
			// Read from the socket
			int bytes;
			// One at a time bytes
			if((bytes = read(sock,buff+bytesRead,1))<0) {
				error("readline read error");
				return(-1);
				}
			if(bytes == 0) {
				buff[bytesRead] = '\0';
				return(bytesRead);	// socket closed - buffer may have some content
				}
			if(buff[bytesRead] == '\n') {
				// found the end
				buff[bytesRead] = '\0';
				break;
				}
			else if(buff[bytesRead] == '\r')
				bytesRead--;		// ick, kill off carriage returns ... damn Windows
			bytesRead += bytes;
			}

		FD_ZERO(&readSet);
		FD_SET(sock,&readSet);
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		errno=0;	// Clear this as we use it as a critical condition for interrupted selects
		}

	if((retVal == -1)&&(errno!=EINTR)) {
		error("readline select error");
		return(-1);
		}

	buff[bytesRead] = '\0';
	if(messageTrace)
		fprintf(stderr,"<%s\n",buff);
	return(bytesRead);
	}


// Read bytes from the socket until a timeout, or the socket closes
// or the buffer is filled
int readBytes(int sock, int getBytes, char *buff, int buffSize, int timeoutSeconds) {
	if(getBytes > buffSize) {

		char msg[1024];
		snprintf(msg,sizeof(msg),"readBytes buffer too small, asked for %d bytes, but only gave %d bytes",
				getBytes,buffSize-1);
		error(msg);
		return(-1);
		}
	int retVal;
	struct timeval tv;
	fd_set readSet;

	FD_ZERO(&readSet);
	FD_SET(sock,&readSet);
	tv.tv_sec = 0;
	tv.tv_usec = 500;

	int bytesRead = 0;
	time_t start = time(NULL);
	while((bytesRead<getBytes)&&(time(NULL) - start < timeoutSeconds)
				&&(((retVal = select(sock+1,&readSet,NULL,NULL,&tv)) != -1))||(errno==EINTR)) {
		if(retVal > 0) {
			// Read from the socket
			int bytes;
			// One at a time bytes
			if((bytes = read(sock,buff+bytesRead,getBytes-bytesRead))<0) {
				error("readBytes read error");
				return(-1);
				}
			if(bytes == 0)
				return(bytesRead);	// socket closed - buffer may have some content
			bytesRead += bytes;
			buff[bytesRead] = '\0';		// Just incase
			}



		FD_ZERO(&readSet);
		FD_SET(sock,&readSet);
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		errno=0;	// Clear this as we use it as a critical condition for interrupted selects
		}

	if((retVal == -1)&&(errno!=EINTR)) {
		error("readBytes select error");
		return(-1);
		}

	if(messageTrace)
		// fprintf(stderr,"ReadBytes [%d]\n",bytesRead);
		fprintf(stderr,"\n%s\n",buff);
	return(bytesRead);
	}


// Write the buffer to the socket - based on fixed size (binary content assumed)
int writeBytes(int sock,const char *buff, int size, int timeoutSeconds) {
	int retVal;
	struct timeval tv;
	fd_set writeSet;

	FD_ZERO(&writeSet);
	FD_SET(sock,&writeSet);
	tv.tv_sec = 0;
	tv.tv_usec = 500;

	int bytesSent = 0;
	time_t start = time(NULL);
	while((bytesSent < size) && (time(NULL) - start < timeoutSeconds)
				&&(((retVal = select(sock+1,NULL,&writeSet,NULL,&tv)) != -1))||(errno==EINTR)) {
		if(retVal > 0) {
			// Write to the socket
			int bytes;
			if((bytes = write(sock,buff+bytesSent,size-bytesSent))<0) {
				error("writeBytes write error");
				return(-1);
				}
			bytesSent += bytes;
			}

		FD_ZERO(&writeSet);
		FD_SET(sock,&writeSet);
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		errno=0;	// Clear this as we use it as a critical condition for interrupted selects
		}

	if((retVal == -1)&&(errno!=EINTR)) {
		error("writeBytes select error");
		return(-1);
		}
	if(messageTrace) {
		int binary=0;
		int x;
		for(x=0;x<size;x++) {
			if((buff[x] != '\n') && (isprint(buff[x])==0))
				binary=1;
			}
		if(binary)
			fprintf(stderr,">%d Bytes sent\n",bytesSent);
		else
			fprintf(stderr,">%*.*s",bytesSent,bytesSent,buff);
		}
	return(bytesSent);
	}



int startListener(int port, int localHostOnly) {

        struct sockaddr_in sa;
        int val;
        int sock;

        if((sock=socket(PF_INET,SOCK_STREAM,0)) < 0) {
                error("socket");
		return(-1);
		}

        val = 1;
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
                error("setsockopt");
		return(-1);
		}

        sa.sin_family = PF_INET;
        sa.sin_port = htons(port);
	if(localHostOnly)
        	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
        	sa.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(sock,(struct sockaddr*)&sa,sizeof(sa)) < 0) {
                error("bind");
		return(-1);
		}

        if(listen(sock,5) < 0) {
                error("listen");
		return(-1);
		}
        return(sock);
        }



//
//
//	UDP "stuff"
//
//


// Bind a UDP socket to a port (you can just do "read" and "write" on the socket.
int startUDPListener(int port) {

	struct sockaddr_in sa;
	int val;
	int sock;

	if((sock=socket(PF_INET,SOCK_DGRAM,0)) < 0)
		error("startUDPListener socket");

	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
		error("startUDPListener setsockopt");

	sa.sin_family = PF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY); 

	if(bind(sock,(struct sockaddr*)&sa,sizeof(sa)) < 0)
		error("startUDPListener bind");

	return(sock);
	}



