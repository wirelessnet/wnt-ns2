#ifndef ns_mudp_h
#define ns_mudp_h

#include "udp.h"

class mUdpAgent : public UdpAgent {
public:
	mUdpAgent();
	virtual void sendmsg(int nbytes, AppData* data, const char *flags = 0);
	virtual int command(int argc, const char*const* argv);
protected:
	int id_;	
	char BWfile[100];
	FILE *BWFile;
	int openfile;
};

#endif
