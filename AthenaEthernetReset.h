#ifndef AthenaEthernetReset_h
#define AthenaEthernetReset_h

#include <Ethernet.h>

#ifndef ETHERNETRESET_DEBUG
#define ETHERNETRESET_DEBUG 0
#endif

#if ETHERNETRESET_DEBUG > 0
#define ETHERNET_DEBUG(c) c
#else
#define ETHERNET_DEBUG(c)
#endif

class AthenaEthernetReset
{
  private:
	EthernetServer* _server;
	EthernetClient _client;
	char _path[20];

	void stdResponse(const char* msg);
	void watchdogReset();
	void stop(void);

  public:
	AthenaEthernetReset(int port);
	~AthenaEthernetReset() = default;

	void begin(void);
	void check(void);
};

#endif
