Faucet-Forwarding
=================

A small wrapper for Gang Garrison 2 of miniupnpc for automatic port redirection

Dependencies
------------

Faucet Forwarding requires the following dependency
[miniupnpc] http://miniupnp.tuxfamily.org/

Building
------------
libminiupnpc.a should be statically linked to the project in order for it to compile.
For the purposes of Game Maker wrapping - it should be exported as a DLL. I've also included
my codeblocks project file, complete with relatively imported dependency folders for use, though
I doubt anyone's actually going to be compiling this other than me

In the configuration I have the following linked (In this order)

-lws2_32
-lminiupnpc
-liphlpapi

Calling
------------
When calling the individual functions, it is important to make sure that you are always passing
strings to upnp_forward_port, and upnp_release_port. For some (odd) reason Game Maker does no type-checking
when dealing with extensions

Functions
------------
upnp_discover(double delay)
Search for Internet Gateway Devices on the network and attempt to see if they support universal
plug and play. This must be called first before attempting to forward a port. Returns 0 if successful,
or an error code otherwise

upnp_forward_port(string internal port, string external port, string protocal, string leaseTime)
Attempts to map a port to your router's port if upnp is availible. Internal port and external
port usually are set to the same port. Protocal is either "TCP", or "UDP". leaseTime is usally set to
"0" (Unlimited).
Returns an error code.

upnp_release_port(string port, string protocal)
Attempts to release the given port of the given protocal

upnp_error_string(double error_code)
Returns a human readable string of the given error code, or a null string if there is no error.