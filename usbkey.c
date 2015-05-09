// These functions concern gathering a list of usbkeys currently plugged into the system
// if you are looking for example PAM module code, then look in pam_module.c



#include "usbkey.h"
#include "utility.h"
#include <glob.h>

#define USB_DEVS "/sys/bus/usb/devices/*"



//The SysFS lines we want to read should only contain one line, so this 
//function just reads the first line from a file
char *SysFSRead1LineFile(char *RetStr, char *Path)
{
FILE *F;

//all strings done on the heap! Strings in sysfs files should not
//be more than 255 bytes
RetStr=(char *) realloc(RetStr, 255+1);
*RetStr='\0';
F=fopen(Path, "r");
if (F)
{
	fgets(RetStr, 255, F);
	StripTrailingWhitespace(RetStr);
	fclose(F);
}

return(RetStr);
}


//This function iterates through the directories in /sys/bus/usb/devices
//and reads the 'bDeviceClass' file in each, looking for devices which have
//class values of either 00 or 08. These are likely to be usb keys. It then
//reads the file 'serial' into a string list of serial numbers
char *LoadActiveUSBSerialNums(char *RetStr)
{
char *Serial=NULL, *Product=NULL, *Tempstr=NULL;
glob_t Glob;
int i, val=-1;

glob(USB_DEVS,0,0,&Glob);
for (i=0; i < Glob.gl_pathc; i++)
{
  Tempstr=MCopyStr(Tempstr,Glob.gl_pathv[i],"/bDeviceClass",NULL);
	Product=SysFSRead1LineFile(Product, Tempstr);

	if (Product) val=atoi(Product);
	else val=-1;

	//device class '8' is mass-storage
	if ((val==0) || (val==8))
	{
  Tempstr=MCopyStr(Tempstr,Glob.gl_pathv[i],"/serial",NULL);
	Serial=SysFSRead1LineFile(Serial, Tempstr);

	if (StrLen(Serial)) RetStr=MCatStr(RetStr,Serial," ",NULL);
	}
}
globfree(&Glob);

Destroy(Tempstr);
Destroy(Serial);
Destroy(Product);

return(RetStr);
}

