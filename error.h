#ifndef _ERROR_H_
#define _ERROR_H_

// IMPORTANT INFORMATION
//
//
// In order to hook centaur's debug stream into the info/warning/error messages
// There are two functions for info/warning/error defined
// These are then #defined to 'info/warning/error'
// Thus centaur gets infoExtended... whereas everyone else gets infoNormal...
//
// The externs (debugSocket, debugSa and debugIndex) are actually created in misc.h/misc.cpp
// which also taps into centaur debug functions for centaur
// They are just externed here so that they will be found on link

#ifdef _EXTENDED_DEBUG_				// Centaur get's extended functions
#define error errorExtended
#define warning warningExtended
#define info infoExtended
#define alert alertExtended
#else						// Everyone else get's normal functions
#define error errorNormal
#define warning warningNormal
#define info infoNormal
#define alert alertNormal
#endif

extern int debugSocket;                         // Used only by centaur debugging

extern struct sockaddr_in *debugSa;             // Used only by centaur debugging

extern long debugIndex;				// Used only by centaur debugging


void init_messages(const char *progname);	// Set the program name for use in messaging

void setSyslogUse(int s);			// 0 means no syslog, anything else turns on syslog events instead of fprintf

// Default is fprintf logging

void errorNormal(const char *fmtMsg, ...);
void warningNormal(const char *fmtMsg, ...);
void infoNormal(const char *fmtMsg, ...);
void alertNormal(const char *fmtMsg, ...);
void errorExtended(const char *fmtMsg, ...);
void warningExtended(const char *fmtMsg, ...);
void infoExtended(const char *fmtMsg, ...);
void alertExtended(const char *fmtMsg, ...);

#endif

