#include "interface.h"
#include "user_interface.h"
#include "osapi.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define MAX_WIFI_STATIONS 50

/*
TODO:
-OK- HTTP server autostart
- Connecting to shoutcast server
- Sending data to uC
- UART semaphores
- Admin panel with in/out data
*/

struct station_config cfg;

uint8_t startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void printInfo(char* s)
{
	int i;
	//char buf[12];
	//sprintf(buf, " %d", size);
	printf("\n#INFO#");
	printf(s);
	//printf(buf);
	//for(i=0; i<size; i++) uart0_putc(*s++);
	printf("\n##INFO#");
}

/*
* LIST OF REQUIRED COMMANDS:
* -OK- List available wifi networks
* -OK- Connect to choosen wifi network (id, passwd)
* -OK- Disconnect from wifi (?)
* -OK- Get info about connected network
* - Set settings (to admin panel)
* - Get settings (from admin panel)
* -OK- Connect to shoutcast (address, path)
* -OK- Disconnect from shoutcast
*/

void wifiScanCallback(void *arg, STATUS status)
{
	if(status == OK)
	{
		int i = MAX_WIFI_STATIONS;
		char buf[64];
		struct bss_info *bss_link = (struct bss_info *) arg;
		printf("\n#WIFI.LIST#");
		while(i > 0)
		{
			i--;
			bss_link = bss_link->next.stqe_next;
			if(bss_link == NULL) break;
			sprintf(buf, "\n%s;%d;%d;%d", bss_link->ssid, bss_link->channel, bss_link->rssi, bss_link->authmode);
			printf(buf);
		}
		printf("\n##WIFI.LIST#");
	}
}

void wifiScan()
{
	wifi_station_scan(NULL, wifiScanCallback);
}

void wifiConnect(char* cmd)
{
	int i;
	
	for(i = 0; i < 32; i++) cfg.ssid[i] = 0;
	for(i = 0; i < 64; i++) cfg.password[i] = 0;
	cfg.bssid_set = 0;
	
	wifi_station_disconnect();
	
	char *t = strstr(cmd, "(\"");
	if(t == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\",\"");
	if(t_end == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	
	strncpy( cfg.ssid, (t+2), (t_end-t-2) );
	
	t = t_end+3;
	t_end = strstr(t, "\")");
	if(t_end == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	
	strncpy( cfg.password, t, (t_end-t)) ;
	
	wifi_station_set_config(&cfg);

	if( wifi_station_connect() ) printf("\n##WIFI.CONNECTED#");
	else printf("\n##WIFI.NOT_CONNECTED#");
}

void wifiDisconnect()
{
	if(wifi_station_disconnect()) printf("\n##WIFI.NOT_CONNECTED#");
	else printf("\n##WIFI.DISCONNECT_FAILED#");
}

void wifiStatus()
{
	struct ip_info ipi;
	char buf[32+50];
	uint8_t t = wifi_station_get_connect_status();	
	wifi_get_ip_info(0, &ipi);
	sprintf(buf, "#WIFI.STATUS#\n%d\n%d.%d.%d.%d\n%d.%d.%d.%d\n%d.%d.%d.%d\n##WIFI.STATUS#\n",
			  t, (ipi.ip.addr&0xff), ((ipi.ip.addr>>8)&0xff), ((ipi.ip.addr>>16)&0xff), ((ipi.ip.addr>>24)&0xff),
			 (ipi.netmask.addr&0xff), ((ipi.netmask.addr>>8)&0xff), ((ipi.netmask.addr>>16)&0xff), ((ipi.netmask.addr>>24)&0xff),
			 (ipi.gw.addr&0xff), ((ipi.gw.addr>>8)&0xff), ((ipi.gw.addr>>16)&0xff), ((ipi.gw.addr>>24)&0xff));
	printf(buf);
}

void wifiGetStation()
{
	char buf[131];
	struct station_config cfgg;
	wifi_station_get_config(&cfgg);
	sprintf(buf, "\n#WIFI.STATION#\n%s\n%s\n##WIFI.STATION#\n", cfgg.ssid, cfgg.password);
	printf(buf);
}

void clientParseUrl(char* s)
{
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
		return;
    }
    char *url = (char*) malloc((t_end-t+1)*sizeof(char));
    if(url != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) url[tmp] = 0;
        strncpy(url, t+2, (t_end-t));
        clientSetURL(url);
        free(url);
    }
}

void clientParsePath(char* s)
{
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
		return;
    }
    char *path = (char*) malloc((t_end-t+1)*sizeof(char));
    if(path != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) path[tmp] = 0;
        strncpy(path, t+2, (t_end-t));
        clientSetPath(path);
        free(path);
    }
}

void clientParsePort(char *s)
{
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
		return;
    }
    char *port = (char*) malloc((t_end-t+1)*sizeof(char));
    if(port != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) port[tmp] = 0;
        strncpy(port, t+2, (t_end-t));
        uint16_t porti = atoi(port);
        clientSetPort(porti);
        free(port);
    }
}

void checkCommand(int size, char* s)
{
	char *tmp = (char*)malloc((size+1)*sizeof(char));
	int i;
	for(i=0;i<size;i++) tmp[i] = s[i];
	tmp[size] = 0;
	if(strcmp(tmp, "wifi.list") == 0) wifiScan(); //printInfo("WIFI LIST");
	else if(startsWith("wifi.con", tmp)) wifiConnect(tmp);
	else if(strcmp(tmp, "wifi.discon") == 0) wifiDisconnect();
	else if(strcmp(tmp, "wifi.status") == 0) wifiStatus();
	else if(strcmp(tmp, "wifi.station") == 0) wifiGetStation();
	/*else if(strcmp(tmp, "srv.start") == 0) serverInit();
    else if(strcmp(tmp, "srv.stop") == 0) serverDisconnect();
    else if(startsWith("cli.url", tmp)) clientParseUrl(tmp);
    else if(startsWith("cli.path", tmp)) clientParsePath(tmp);
    else if(startsWith("cli.port", tmp)) clientParsePort(tmp);
    else if(strcmp(tmp, "cli.start") == 0) clientConnect();
    else if(strcmp(tmp, "cli.stop") == 0) clientDisconnect();*/
	else printInfo(tmp);
	free(tmp);
}

void printConfig()
{
	
}