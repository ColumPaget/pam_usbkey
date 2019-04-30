# PAM USBKEY

This is a simple PAM (Pluggable Authentication Modules) Module written as a learning exercise. The code is commented in the hope of providing a template for others to follow. It provides a "poor person's two-factor authentication" feature by checking that a pre-registered usb key is plugged into the system. Each usb-drive should have a unique serial number, and this module checks if a pre-registered drive is present. By editing the PAM configuration this module can be used to require a usb key be present in combination with the normal password in order to allow login, or su, or any other PAM aware service. It can also be used to provide passwordless login when a usb key is plugged in (which is probably crazy).



# BIG FAT WARNING

Firstly, you should be aware that changing your PAM configuration could result in locking yourself out of your own computer systems if you get something wrong or  encounter some kind of weird error. If you forget to supply a key= option in the config, then you will have effectively created a 'deny' rule Thus you should first test this module on the 'su' configuration on a non-essential machine that allows root login on the vitual terminals tty1 tty2 ... ttyN. Thus, if something goes wrong, you'll still be able to log in as root and correct it.

Be aware that any user already logged into a linux system can see what usb-keys are plugged in by looking in the `/sys/bus/usb/devices` directories. Of course, this information may not be much use to them, because they'd have to go away and create a key with a matching serial number, and plug it into the computer, and if they've got physical access to the machine there may be much easier ways for them to log into it. However, this is something to consider if you're looking to use this module as an extra security feature to, for instance, protect 'su'.

This module is best used in combination with password authentication. Although passwordless login is possible if so configured, it is really provided as a demonstration feature.

The serial keys are stored in the pam configuration files (normally found in `/etc/pam.d/`), and thus it might be a good idea to ensure these files are not world-readable. Though, again, this information may not be that useful to an attacker, who still has to plug in a matching usb key, or find some way of fooling the system into thinking one is plugged in when it isn't.

This PAM module is free software under the Gnu Public Licence version 3,  and comes with no express or implied warranties or guarentees of anything. 



# INSTALL

The usual proceedure:

```
./configure
make
make install
```

should work. The 'make install' stage will have to be done as root. This will copy the pam_usbkey.so file into /lib/security.



# CONFIGURATION

pam_usbkey.so is configured by adding a line to the appropriate file in /etc/pam.d. So, for example, if we wish to add pam_usbkey to the 'su' service, we would add the following line to /etc/pam.d/su

```
auth    required  pam_usbkey.so user=root tty=!tty?,tty3,tty4 key=123123332123421
```

This specifies that the pam_usbkey.so module is *required* for authentication, where the user (that we are trying to log in as or switch to) is root and the tty is either tty3, tty4, or any tty that is not of the form tty? (where tty? is an fnmatch pattern). The key with serial number 123123332123421 must be plugged into the system for authentication to continue.

the first three fields here are standard PAM config items. Field 1 states the type of PAM operation, in this case authentication. Field 2 can be 'required' or 'sufficient', the first meaning that this PAM operation must return success for login to occur, but that it alone is not enough to allow login. If this rule is marked 'sufficient' instead, then on it's own it's enough to allow login, which in our case would mean that the presence of a USB key would allow login without a password. The third field is simply the name of the PAM module used in this rule, which in this case is the 'pam_usbkey.so' module.

All fields after the module name are arguments that are passed to the module. In the case of pam_usbkey.so these arguments are comma-separated lists of fnmatch (Shell-style) patterns that specify users, ttys, usb-keys or remote hosts. If the tty or rhost options are not supplied, or are blank, then they are considered as not applying. This means that if you leave tty= blank, or miss it out, that no local tty will have to pass the rule. Similarly, if rhosts is left out or is empty, then no remote host will have to pass the rule.

Many services do not supply at tty or rhost option anyways, making one of these options irrelevant for the service. For instance, 'su' does not seem to supply an rhosts value on my system, hence these two rules only apply to authentication that has a concept of a remote host or tty. However, the rules may be used in conjuction, and where they do not apply, they will be ignored.

Possible pam_usbkey.so arguments are:

	* user=[ user list ]   List of users to whom this rule applies
	  users=[ user list ]

	* tty=[ tty list ]   List of ttys to which this rule applies
	  ttys=[ tty list ]

	* rhost=[ host list ]   List of hosts (IP addresses) to which this rule applies
	  rhosts=[ host list ]

	* key=[key list]   List of usb-key serial numbers that can be used to authenticate this rule
	  keys=[key list]

Usb key serial numbers can be found in the 'serial' files in the usb device directories under /sys/bus/usb/devices. pam_usbkey.so comes with a tiny shell-script, 'listusbkeys.sh' that prints out a list of serial numbers of devices currently plugged in.



# EXAMPLES

Require a usbkey for all remote hosts. As tty= is not supplied, a usbkey is not needed for local login
```
auth required pam_usbkey.so user=* rhosts=* key=12345678910
```

If alice's usb-key is plugged in, then she can log into tty1-4 without a password (sufficient). As an rhosts= option is not supplied, this rule would not apply to any remote host
```
auth sufficient pam_usbkey.so user=alice tty=tty[1-4] key=75957437595
```

Require a usbkey for users eve and mallory if they are logging in from any host that is not on the `192.168.*.*` subnet. A choice of two keys is available.
```
auth required pam_usbkey.so user=eve,mallory rhosts=!192.168.*.* key=12345678910,888383838
```

Require a usbkey for all users who are not bob
```
auth required pam_usbkey.so user=!bob rhosts=* tty=* key=12345678910,888383838
```

Require a usbkey for all users whose names start with 'dev', and also for 'dev12345'
```
auth required pam_usbkey.so user=!dev*,dev12345 rhosts=* tty=* key=12345678910,888383838
```

Require a usbkey for user eve to log in locally. The lack of an rhosts implies that this rule is used in a service that doesn't support remote login, but this is eve so who knows what might be going on here?
```
auth required pam_usbkey.so user=eve tty=* key=75957437595
```
