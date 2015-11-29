#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DEBUG_BUFSIZE 256

extern int errno;


#include "error.h"

static const char *progName="unknown";

static int sysLogState = 0;		// !0 = use syslog, 0 = don't use syslog (default)

// Code is written to be able to run syslog or fprintf style
void init_messages(const char *p)
{
	// If necessary, open syslog (always open, even if we don't use it)
	openlog(p,LOG_ODELAY|LOG_PID, LOG_DAEMON|LOG_NOTICE);
	progName = p;
}

void setSyslogUse(int s)

{
	sysLogState=s;		// !0 = use syslog, 0 = don't use syslog
}


// A standard version of doMessage, used by all non-centaur apps
void doMessage(int facility,const char *msgType,const char *fmtMsg, va_list *ap)
{
	if (sysLogState)
		vsyslog(facility,fmtMsg,*ap);
	else {
		fprintf(stderr,"(%s) %s: ",progName, msgType);
		vfprintf(stderr,fmtMsg,*ap);
		fprintf(stderr,"\n");
	}

	if (errno != 0) {
		if (sysLogState)
			syslog(facility,"ERRNO %s", strerror(errno));
		else {
			fprintf(stderr,"(%s) ERRNO %s",progName, strerror(errno));
			fprintf(stderr,"\n");
		}

		errno = 0;	// clear for the next round
	}
}



// These three functions are defined to 'error','warning','info' respecitvely
// for Non-CENTAUR compile only
void errorNormal(const char *fmtMsg, ...)
{
	va_list ap;
	va_start(ap, fmtMsg);
	doMessage(LOG_DAEMON|LOG_ERR,"ERROR",fmtMsg,&ap);
	va_end(ap);
}

void warningNormal(const char *fmtMsg, ...)
{
	va_list ap;
	va_start(ap, fmtMsg);
	doMessage(LOG_DAEMON|LOG_WARNING,"WARNING",fmtMsg,&ap);
	va_end(ap);
}

void infoNormal(const char *fmtMsg, ...)
{
	va_list ap;
	va_start(ap, fmtMsg);
	doMessage(LOG_DAEMON|LOG_INFO,"INFO",fmtMsg,&ap);
	va_end(ap);
}

void alertNormal(const char *fmtMsg, ...)
{
	va_list ap;
	va_start(ap, fmtMsg);
	doMessage(LOG_DAEMON|LOG_ALERT,"ALERT",fmtMsg,&ap);
	va_end(ap);
}


