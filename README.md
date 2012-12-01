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