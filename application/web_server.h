#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "hts221.h"

int WebServer_Start(void);
int WebServer_Poll(const HTS221_Sample *sample);

#endif
