#ifndef _MISC_H_
#define _MISC_H_


// Tool to parse a string that has single character separators and possible escapes of the separator characters
// sep		the separator char
// ptrs		is an array of char pointers that will be filled with pointers into the parsed line
// numPtrs	is the number of pointers in ptrs
// line		is the buffer that is NUL byte terminated that is to be parsed. NOTE, nul-bytes are put in place
//              for the separators
// returned is the number of items found.
int parseArgs(char sep, char **ptrs,int numPtrs, char *line);


// Debug tools
extern int funcDebug;
extern int funcDepth;
extern int debugSocket;					// Used only by centaur
extern struct sockaddr_in *debugSa;			// Used only by centaur

// debugFunction extends our debug macros and is only used by centaur
extern void debugFunction(int depth, int action, const char *func, int line, const char *message);

#define DEBUGMSG(func,message)          if(funcDebug) { printDepth(funcDepth+1,' ');  fprintf(stderr," %s(%d): '%s'\n",func,__LINE__,message); }
#define DEBUGENTER(func,message)        if(funcDebug) { printDepth(++funcDepth,'+');  fprintf(stderr," %s(%d) - (%s)\n",func,__LINE__,message); }
#define DEBUGEXIT(func,message)         if(funcDebug) { printDepth(funcDepth--,'-');  fprintf(stderr," %s(%d) - (%s)\n",func,__LINE__,message); }
void printDepth(int x,char c);

#endif
