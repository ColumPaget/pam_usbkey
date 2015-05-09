// These functions concern gathering a list of usbkeys currently plugged into the system
// if you are looking for example PAM module code, then look in pam_module.c

#ifndef USBKEYS_H
#define USBKEYS_H

#include "common.h"

char *LoadActiveUSBSerialNums(char *RetStr);

#endif
