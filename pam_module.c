#include "common.h"
#include "usbkey.h"
#include "utility.h"
#include <syslog.h>

//Define which PAM interfaces we provide. In this case we are
//only going to provide an authentication interface, i.e. one 
//that decides if a login in allowed or not
#define PAM_SM_AUTH

// We do not supply these
/*
#define PAM_SM_ACCOUNT
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION
*/

// Include PAM headers 
#include <security/pam_appl.h>
#include <security/pam_modules.h>



int CheckAuthItem(const char *KeyList, const char *ActiveUSBKeys, const char *User, const char *TTY, const char *RHost)
{
char *MatchList=NULL;
const char *ptr;
int UserMatch=FALSE, KeyMatch=FALSE, TTYMatch=FALSE, HostMatch=FALSE;

ptr=GetTok(KeyList, ' ', &MatchList);
while (ptr)
{
	if (strncmp(MatchList,"rhost=",6)==0) HostMatch=ItemMatches(RHost, MatchList+6);
	if (User)
	{
		if (strncmp(MatchList,"user=",5)==0) UserMatch=ItemMatches(User, MatchList+5);
		if (strncmp(MatchList,"users=",6)==0) UserMatch=ItemMatches(User, MatchList+6);
	}
	if (TTY)
	{
		if (strncmp(MatchList,"tty=",4)==0) TTYMatch=ItemMatches(TTY, MatchList+4);
		if (strncmp(MatchList,"ttys=",5)==0) TTYMatch=ItemMatches(TTY, MatchList+5);
	}
	if (KeyList)
	{
		if (strncmp(MatchList,"key=",4)==0) KeyMatch=ItemListMatches(ActiveUSBKeys, MatchList+4);
		if (strncmp(MatchList,"keys=",5)==0) KeyMatch=ItemListMatches(ActiveUSBKeys, MatchList+5);
	}
ptr=GetTok(ptr, ' ', &MatchList);
}

Destroy(MatchList);

if (UserMatch && (TTYMatch || HostMatch)) 
{
	if (! KeyMatch) return(PAM_PERM_DENIED);
	return(PAM_SUCCESS);
}

return(PAM_IGNORE);
}


int CheckAuth(const char *KeyList, const char *ActiveUSBKeys, const char *User, const char *TTY, const char *RHost)
{
char *KeyInfo=NULL;
const char *ptr;
int result=PAM_IGNORE;


ptr=GetTok(KeyList, '|', &KeyInfo);
while (ptr)
{
	StripTrailingWhitespace(KeyInfo);
	StripLeadingWhitespace(KeyInfo);
	result=CheckAuthItem(KeyInfo, ActiveUSBKeys, User, TTY, RHost);
	ptr=GetTok(ptr, '|', &KeyInfo);
}

Destroy(KeyInfo);

return(result);
}


// PAM entry point for authentication. This function gets called by pam when
//a login occurs. argc and argv work just like argc and argv for the 'main' 
//function of programs, except they pass in the options defined for this
//module in the pam configuration files in /etc/pam.conf or /etc/pam.d/
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{
	char *Tempstr=NULL, *KeyList=NULL;
	char *ActiveUSBKeys=NULL;
	const char *ptr;
	int result=PAM_IGNORE, i;

	//These are defined as 'const char' because they passwd to us from the parent
	//library. When we called pam_get_<whatever> the pam library passes pointers
	//to strings in it's own code. Thus we must not change or free them
	const char *pam_user = NULL, *pam_tty=NULL, *pam_rhost=NULL;

	//get the user. If something goes wrong we return PAM_IGNORE. This tells
	//pam that our module failed in some way, so ignore it. Perhaps we should
	//return PAM_PERM_DENIED to deny login, but this runs the risk of a broken
	//module preventing anyone from logging into the system!
	if (pam_get_user(pamh, &pam_user, NULL) != PAM_SUCCESS) return(PAM_IGNORE);
	if (pam_user == NULL) return(PAM_IGNORE);

	//perhaps there will not be a tty if we are logging in remotely
	pam_get_item(pamh, PAM_TTY, (const void **) &pam_tty);
	if (! pam_tty) pam_tty="";

	//perhaps there will not be a remote host if we are logging in locally
	pam_get_item(pamh, PAM_RHOST, (const void **) &pam_rhost);
	if (! pam_rhost) pam_rhost="";


	for (i=0; i < argc; i++)
	{
		ptr=argv[i];

		if (strncmp(ptr,"key=",4)==0)   Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
		if (strncmp(ptr,"keys=",5)==0)  Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
		if (strncmp(ptr,"tty=",4)==0)   Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
		if (strncmp(ptr,"ttys=",5)==0)  Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
		if (strncmp(ptr,"user=",5)==0)  Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
		if (strncmp(ptr,"users=",6)==0) Tempstr=MCatStr(Tempstr, ptr, " ", NULL);
	}

	if (StrLen(Tempstr)) KeyList=MCopyStr(KeyList, Tempstr, "|", NULL);


	ActiveUSBKeys=LoadActiveUSBSerialNums(ActiveUSBKeys);
	result=CheckAuth(KeyList, ActiveUSBKeys, pam_user, pam_tty, pam_rhost);

	Destroy(ActiveUSBKeys);
	Destroy(Tempstr);
	Destroy(KeyList);

  return(result);
}


//We do not provide any of the below functions, we could just leave them out
//but apparently it's considered good practice to supply them and return
//'PAM_IGNORE'

//PAM entry point for starting sessions. This is called after a user has 
//passed all authentication. It allows a PAM module to perform certain tasks
//on login, like recording the login occured, or printing a message of the day
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{
	return(PAM_IGNORE);
}


//PAM entry point for ending sessions. This is called when a user logs out
//It allows a PAM module to perform certain tasks on logout
//like recording the logout occured, or clearing up temporary files
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return(PAM_IGNORE);
}

// PAM entry point for 'account management'. This decides whether a user
// who has already been authenticated by pam_sm_authenticate should be
// allowed to log in (it considers other things than the users password)
// Really this is what we should have used here
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv){
	return(PAM_IGNORE);
}


//PAM entry point for setting 'credentials' or properties of the user
//If our module stores or produces extra information about a user (e.g.
//a kerberous ticket or geolocation value) then it will pass this information
//to a PAM aware program in this call
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{
	return(PAM_IGNORE);
}

// PAM entry point for changing passwords. If our module stores passwords
// then this will be called whenever one needs changing
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return(PAM_IGNORE);
}


//I couldn't find any documentation on this. I think it notifies PAM of our
//module name
#ifdef PAM_MODULE_ENTRY
PAM_MODULE_ENTRY("pam_usbkey");
#endif
