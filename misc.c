#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "misc.h"
#include <stdio.h>
#include <string.h>

#define DEBUG_BUFSIZE 512

int funcDebug = 0;           // Local parsing debug
int funcDepth = 0;
int debugSocket = 0;
long debugIndex=0;

struct sockaddr_in *debugSa;

void printDepth(int x,char c)
{
	int t;

	for (t=0;t<x;t++)
		fputc(c,stderr);
}


int parseArgs(char sep, char **ptrs,int numPtrs, char *line)
{

	memset(ptrs,0,numPtrs*sizeof(char*));		// zero out the resulting list of pointers for safey
	int count=0;

	if (line) {
		char *s = line;

		while ((*s != '\0')&&(count<numPtrs)) {
			ptrs[count++] = s;

			while ((*s != '\0')&&(*s != sep)) {
				if ((*s == '\\')&&*(s+1))
					s++;	// skip over the \ to protect the exscaped char from

				// being used in the parsing
				s++;
			}

			if (*s == sep)
				*s++ = '\0';
		}
	}

	return(count);
}

// This function supplements the DEBUGENTER/EXIT/MSG macros only on centaur
// This allows for streaming of debug logs over UDP

void debugFunction(int depth, int action, const char *func, int line, const char *message)
{
	char debugBuf[DEBUG_BUFSIZE];
	char debugBuf2[DEBUG_BUFSIZE];
	int t;
	char c;

	// If we get kicked off mid-way through execution our depth will be off. This will get it back in order

	if (depth<=0) depth=3;

	// In case someone forgot to put a DEBUGEXIT in where needed, we may start to creep up on our depth
	// To keep it useful (especially over UDP with our limited size buffer) we'll take action here..
	if (depth>40) depth=15;

	// Determine symbol based on action code
	switch (action) {

		case 1 :
			c=' ';

			break;

		case 2 :
			c='+';

			break;

		case 3 :
			c='-';

			break;
	}

	// Pad out the start of the line as appropriate
	for (t=0;t<depth;t++) debugBuf[t]=c;

	debugBuf[t]=0;

	// Use formatting appropriate to action code
	switch (action) {

		case 1 : // MSG
			snprintf(&debugBuf[t], DEBUG_BUFSIZE-t-1, "%s(%d): '%s'\n", func, line, message);

			break;

		case 2 : // ENTER

		case 3 : // EXIT
			snprintf(&debugBuf[t], DEBUG_BUFSIZE-t-1, "%s(%d) - (%s)\n", func, line, message);

			break;
	}

	debugBuf[DEBUG_BUFSIZE-1]=0;		// Be sure we null terminate

	// STDERR output if centaur is started with -d

	if (funcDebug) fprintf(stderr, "%s", debugBuf);

	// UDP output if it's been turned on
	if (debugSocket>0) {
		snprintf(debugBuf2, DEBUG_BUFSIZE-1, "DBG:%ld:%s", debugIndex++, debugBuf);
		debugBuf2[DEBUG_BUFSIZE-1]=0;

		sendto(debugSocket, debugBuf2, strlen(debugBuf2), 0, (struct sockaddr *) debugSa, sizeof (struct sockaddr_in));
	}
}

