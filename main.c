#include "connectionTools.h"
#include "error.h"



int readWrite(int readSock, int writeSock) {
        char message[2048];
        int size=read(readSock,message,sizeof(message));
        if(size > 0) {
                if(write(writeSock, message, size) != size)
                        error("Short Write or Failure",__FILE__,__LINE__);
                }
        else {
                if(size == 0) {
                        // Normal case
                        fprintf(logFp,"Normal Exit Conditions for sockets Read=%d write=%d\n", readSock, writeSock);
                        // if(shutdown(writeSock,SHUT_RDWR)!=0)
                        if(close(writeSock)!=0)
                                error("close error",__FILE__,__LINE__);
                        // if(shutdown(readSock,SHUT_RDWR)!=0)
                        if(close(readSock)!=0)
                                error("Close error",__FILE__,__LINE__);
                        exit(0);        // exit ... this causes all open sockets to close on exit
                        }
                else
                        error("Socket Read Failure",__FILE__,__LINE__);
                }
        return(size);
        }



int runPassthrough(int sock, char *ip, int port) {
	DEBUGENTER("runPassthrough",NULL);
	int server;
        if((server=connectToServer(ip, atoi(port))) != -1) { // Connect
                int retVal, fdMax;
                struct timeval tv;
                fd_set readSet;

                DEBUGMSG("runPassthrough","Connected\n");

                FD_ZERO(&readSet);
                FD_SET(sock,&readSet);
                FD_SET(server,&readSet);
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                fdMax = sock;
                if(server > sock)
                        fdMax = server;

                while((retVal = select(fdMax+1,&readSet,NULL,NULL,&tv)) != -1) {
                        if(FD_ISSET(sock,&readSet))
                                readWrite(sock,server);         // EOF cause process death and socket closure

                        if(FD_ISSET(server,&readSet))
                                readWrite(server, sock);                // EOF cause process death and socket closure

                        FD_ZERO(&readSet);
                        FD_SET(sock,&readSet);
                        FD_SET(server,&readSet);
                        tv.tv_sec = 1;
                        tv.tv_usec = 0;
                        }
                }
	DEBUGEXIT("runPassthrough",NULL);
	}









const char *progname = NULL;

void usage(const char *s, ...) {

        if(s) {
                va_list ap;
                va_start(ap, s);
                fprintf(stderr,"ERROR: ");
                vfprintf(stderr,s, ap);
                fprintf(stderr,"\n");
                va_end(ap);
                }

        fprintf(stderr,"\nusage:\n");
        fprintf(stderr,"\t%s -p <listen port#> -w <web port#> -a <web ip> \n",progname);
        fprintf(stderr,"\n");
        fprintf(stderr,"\t\t-p sets the listening port\n");
        fprintf(stderr,"\t\t-a sets the remote connection ip address\n");
        fprintf(stderr,"\t\t-w sets the remote connection target port\n");

        fprintf(stderr,"\n");
        exit(-1);
        }




int main(int argc, char *argv[]) {

	progname = argv[0];
	init_messages(progname);
	setSyslogUse(0);	// no syslog for now

        // Options set by the command line
        char *fileOverride = NULL;
        int port = 0;
        // Parse the command line
        int x;
        for(x=1;x<argc;x++) {
                if(argv[x][0] == '-') {
                        switch(argv[x][1]) {
                                case 'd' :
                                        funcDebug=1;
                                        break;
                                case 'p' :
                                        if(x+1 >= argc)
                                                usage("missing arg for -f option");
                                        port = atoi(argv[++x]);         // consume the option
                                        break;
                                case 'f' :
                                        if(x+1 >= argc)
                                                usage("missing arg for -f option");
                                        fileOverride = argv[++x];       // consume the option
                                        break;
                                default :
                                        usage("unknown flag -%c",argv[x][1]);
                                }
                        }
                else
                        usage("unflagged option on command line [%s]",argv[x]);
                }




	}

