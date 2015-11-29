#ifndef _CONNECTION_TOOLS_
#define _CONNECTION_TOOLS_

// Connection the provided server ... short timeout is expected
int connectToServer(const char *hostname, int port, int timeoutSeconds);

// Read bytes from the socket until the timeout, or the socket closes
// or the buffer is filled, or a newline is found (and eatin)
int readLine(int sock, char *buff, int buffSize, int timeoutSeconds);

// Read raw bytes from the socket
int readBytes(int sock, int getBytes, char *buff, int buffSize, int timeoutSeconds);
// Write the buffer to the socket - based on fixed size (binary content assumed)
int writeBytes(int sock,const char *buff, int size, int timeoutSeconds);

// Start a listening socket
int startListener(int port, int loadHostOnly);

// Bind a UDP socket to a port (you can just do "read" and "write" on the socket.
int startUDPListener(int port);

// Turn on Message debug output
void setMessageTrace(int i);
extern int messageTrace;


#endif
