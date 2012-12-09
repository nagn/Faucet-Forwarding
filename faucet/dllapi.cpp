#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <miniwget.h>
#include <miniupnpc.h>
#include <upnpcommands.h>
#include <upnperrors.h>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <string>
#define DLLEXPORT extern "C" __declspec(dllexport)
struct UPNPDev * devlist = 0;
char lanaddr[64];	/* my ip address on the LAN */
const char * rootdescurl = 0;
const char * multicastif = 0;
const char * minissdpdpath = 0;
const char * description = "Faucet Forwarding Extension";
int retcode = 0;
int error = 0;
std::stringstream output_error_string;
int ipv6 = 0;
bool debugMode = false;
struct UPNPDev * device;
struct UPNPUrls urls;
struct IGDdatas data;
void MsgBox(const char* caption,const char* message) {
        MessageBox(0, message, caption, MB_OK);
}
const char * protofix(const char * proto)
{
	static const char proto_tcp[4] = { 'T', 'C', 'P', 0};
	static const char proto_udp[4] = { 'U', 'D', 'P', 0};
	int i, b;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_tcp[i])
		          || (proto[i] == (proto_tcp[i] | 32)) );
	if(b)
		return proto_tcp;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_udp[i])
		          || (proto[i] == (proto_udp[i] | 32)) );
	if(b)
		return proto_udp;
	return 0;
}
DLLEXPORT double upnp_set_description(const char * desc){
        description = desc;
        return(0);
}
DLLEXPORT double upnp_discover(double delay){
    //initialize winsock
    WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(nResult != NO_ERROR)
	{
		fprintf(stderr, "WSAStartup() failed.\n");
		return (2);
	}
    if( rootdescurl
	  || (devlist = upnpDiscover(delay, multicastif, minissdpdpath,
	                             0/*sameport*/, ipv6, &error)))
	{
        if(devlist)
        {
            printf("List of UPNP devices found on the network :\n");
            for(device = devlist; device; device = device->pNext)
            {
                printf(" desc: %s\n st: %s\n\n",
                       device->descURL, device->st);
            }
        }
        else
        {
            printf("upnpDiscover() error code=%d\n", error);
        }
        int i = 1;
        if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, &urls, &data, lanaddr, sizeof(lanaddr)))
          || (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))))
        {
            switch(i) {
            case 1:
                printf("Found valid IGD : %s\n", urls.controlURL);
                break;
            case 2:
                printf("Found a (not connected?) IGD : %s\n", urls.controlURL);
                printf("Trying to continue anyway\n");
                break;
            case 3:
                printf("UPnP device found. Is it an IGD ? : %s\n", urls.controlURL);
                printf("Trying to continue anyway\n");
                break;
            default:
                printf("Found device (igd ?) : %s\n", urls.controlURL);
                printf("Trying to continue anyway\n");
            }
            printf("Local LAN ip address : %s\n", lanaddr);
        } else {
            fprintf(stderr, "No valid UPNP Internet Gateway Device found.\n");
            retcode = 3;
        }
        freeUPNPDevlist(devlist); devlist = 0;

	} else {
        fprintf(stderr, "No IGD UPnP Device found on the network !\n");
		retcode = 4;
	}
    return (retcode);
}

DLLEXPORT double upnp_forward_port(const char * iport,
                                    const char * eport,
                                    const char * proto,
                                    const char * leaseDuration)
{
    const char *iaddr = lanaddr;
    char externalIPAddress[40];
	char intClient[40];
	char intPort[6];
	char duration[16];
	int r;

	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "invalid protocol\n");
		return (5);
	}

	UPNP_GetExternalIPAddress(urls.controlURL,
	                          data.first.servicetype,
							  externalIPAddress);
	if(externalIPAddress[0]) {
		printf("ExternalIPAddress = %s\n", externalIPAddress);
	} else {
		printf("GetExternalIPAddress failed.\n");
    }
	r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
	                        eport, iport, iaddr, description, proto, 0, leaseDuration);
	if(r!=UPNPCOMMAND_SUCCESS){
	    output_error_string << "AddPortMapping(" << eport << "," << iport << "," << iaddr << ") failed with code " << r << "(" << strupnperror(r) << ")" << std::endl;
		printf(output_error_string.str().c_str());
        return(6);
        }
	r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
	                                 data.first.servicetype,
    	                             eport, proto,
									 intClient, intPort, NULL/*desc*/,
	                                 NULL/*enabled*/, duration);
	if(r!=UPNPCOMMAND_SUCCESS) {
	    output_error_string << "GetSpecificPortMappingEntry() failed with code " << r << "(" << strupnperror(r) << ")" << std::endl;
		printf(output_error_string.str().c_str());
        return(7);
    }
	if(intClient[0]) {
		printf("InternalIP:Port = %s:%s\n", intClient, intPort);
		printf("external %s:%s %s is redirected to internal %s:%s (duration=%s)\n",
		       externalIPAddress, eport, proto, intClient, intPort, duration);
	}
	return (0);
}


DLLEXPORT double upnp_release_port(const char * eport, const char * proto) {
	int r;
	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "protocol invalid\n");
		return (5);
	}
	r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, eport, proto, 0);
	printf("UPNP_DeletePortMapping() returned : %d\n", r);
	return (0);
}
DLLEXPORT const char * upnp_error_string(double error_code){
    std::stringstream error_string;
    switch((int) error_code){
        case (0):
            error_string << "";
            break;
        case (1):
            error_string << "";
            break;
        case (2):
            error_string << "Unable to start winsock!" << std::endl;
            break;
        case (3):
            error_string << "No valid UPNP Internet Gateway Device found." << std::endl;
            break;
        case (4):
           error_string << "No IGD UPnP Device found on the network !" << std::endl;
           break;
        case (5):
           error_string << "Invalid protocal" << std::endl;
           break;
        case (6):
            error_string << output_error_string.str() << std::endl;
            break;
        case (7):
            error_string << output_error_string.str() <<  std::endl;
            break;
        case (8):
            error_string << "AddPortMapping failed! Action not authorized (606)" << std::endl;
            break;
        case (9):
            error_string << "AddPortMapping failed! ConflictInMappingEntry (718)" << std::endl;
            break;
        case (10):
            error_string << "AddPortMapping failed! OnlyPermanentLeasesSupported (725)" << std::endl;
            break;
        case (11):
            error_string << "AddPortMapping failed! NoPortMapsAvailable (728)" << std::endl;
            break;
        case (12):
            error_string << "AddPortMapping failed! ConflictWithOtherMechanisms (729)" << std::endl;
            break;
        default:
           error_string << "Undefined error code! Error code: " << error_code << std:: endl;
           break;
    }
    return(error_string.str().c_str());
}
DLLEXPORT double dllStartup() {
	return (0);
}

DLLEXPORT double dllShutdown() {
    FreeUPNPUrls(&urls);
	return (0);
}
/*
int main(void){
    double error_code;
    error_code = upnp_discover(2000);
    if (error_code <= 0) {
        error_code = upnp_forward_port("8191", "8191", "TCP", "0");
        upnp_release_port("8191", "TCP");
    }
    printf(upnp_error_string(error_code));

    Sleep(5000);

    return (0);
}
*/
