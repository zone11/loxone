// Philips HUE Steuerung
// (c) 2013 by Romy Glauser
// (c) 2015 Erweiterung durch Andreas Lackner-Werner um LUMITECH (rgbw) & Gruppenfunktionen sowei generelles Cleanup des Codes
// (c) 2015 Erweiterung durch Sven Thierfelder um cx/cy Farbsteuerung für hue-bulbs sowie Glühlampensimulation
// (c) 2018 Changes by RvESoft Solutions for Hue Brige 1.23 version
// (c) 2018 Changes by Christian Egger for better network timing

int DEBUG_LEVEL=0;
int DIMMER_MIN=15;  
int DIMMER_MAX=100;
int DIMMER_SIMMIN=154; //154 = 6400k
int DIMMER_SIMMAX=500; //370 = 2700k

// Übergangszeit von einem Dimmwert zum nächsten (10 = 1 Sekunde)
int TRANSITION_TIME=10;

// Bitte folgende Website:
// http://www.developers.meethue.com/documentation/getting-started 
// beachten um einen gültigen User-Namen zu generieren!
char* IP_ADDRESS = "0.0.0.0";
char* PORT = "80";
char* USERNAME = "blah";


int inputType[12]; 

// Hier die Funktion des Eingangs definieren:
// 0 = RGB-Eingang (Eingang LoxoneFormat, 9-stellige Zahl die RGB codiert mit 100100100 für weiß. Ansteuerung der Lampe via Hue/Sat/Bri
// 1 = Dimmereingang. Eingangswert muss im Bereich DIMMER_MIN / DIMMER_MAX sein. Ansteuerung der Lampe via Bri.
// 2 = ON/OF - Eingang (z.B. für Steckdosen-Adapter)
// 4 = RGB-Eingang (wie '0', aber ansteuerung der Lampe via X/Y/Bri)
// 5 = Dimmereingang (wie '1', Ansteuerung der Lampe via Bri/Ct, Glühlampensimulation)

// Wenn Lampengruppen bestehen, dann diese Typen verwenden:
// 3 = RGB-Eingang (wie '0', aber steuert Lampengruppe via Hue/Sat/Bri)
// 6 = RGB-Eingang (wie '0', aber steuert Lampengruppe via X/Y/Bri)
// 7 = Dimmereingang, wie 1, aber für Gruppe

inputType[0] = 0;  
inputType[1] = 0;
inputType[2] = 0;
inputType[3] = 0;
inputType[4] = 7;
inputType[5] = 3;
inputType[6] = 3;
inputType[7] = 3;
inputType[8] = 3;
inputType[9] = 3;
inputType[10] = 3;
inputType[11] = 3;

int lightOrGroupID[12];   

// Zuweisung der Lampen- oder GruppenIDs. 
// Definiert welcher Eingang des Bausteins welche Lampe bzw. Gruppe ansteuert:
lightOrGroupID[0] = 1;  
lightOrGroupID[1] = 2;
lightOrGroupID[2] = 3;
lightOrGroupID[3] = 4;
lightOrGroupID[4] = 5;
lightOrGroupID[5] = 6;
lightOrGroupID[6] = 7;
lightOrGroupID[7] = 8;
lightOrGroupID[8] = 9;
lightOrGroupID[9] = 10;
lightOrGroupID[10] = 11;
lightOrGroupID[11] = 12;

// Ende der Konfiguration...


char streamname[100];
sprintf(streamname, "/dev/tcp/%s/%s/", IP_ADDRESS, PORT);

int nEvents;

int LIGHT = 1;
int GROUP = 2;

void updateLamp(int idx, float value) 
{
	if (inputType[idx] == -1) 
	{
		//ignorieren
	} else if (inputType[idx] == 0 || inputType[idx] == 4) 
	{   // Lampe Bri/Hue/Sat or X/Y/Bri 
		if (value < 200000000) 
		{ // RGB
			if (inputType[idx] == 0) 
			{
				setColorBHS(lightOrGroupID[idx], value, LIGHT);
			} else if (inputType[idx] == 4) {
				setColorXYB(lightOrGroupID[idx], value, LIGHT);
			}
		} else { // LUMITECH
			setCtBri(lightOrGroupID[idx], value, LIGHT);
		}
	} else if (inputType[idx] == 1) 
		{
		setBrightness(lightOrGroupID[idx], value, LIGHT); 
	} else if (inputType[idx] == 7) 
		{
		setBrightness(lightOrGroupID[idx], value, GROUP); 
	} else if (inputType[idx] == 2) 
		{
		setOnOff(lightOrGroupID[idx], value);
	} else if (inputType[idx] == 3 || inputType[idx] == 6) 
		{  // Gruppe Bri/Hue/Sat or X/Y/Bri 
		if (value < 200000000) 
		{ // RGB
			if (inputType[idx] == 3) 
			{
				setColorBHS(lightOrGroupID[idx], value, GROUP);
			} else if (inputType[idx] == 6) 
			{
				setColorXYB(lightOrGroupID[idx], value, GROUP);
			}
		} else { // LUMITECH
				setCtBri(lightOrGroupID[idx], value, GROUP);
				}
		} else if (inputType[idx] == 5) {
		setBrightnessAsBulb(lightOrGroupID[idx], value);
	}
}

void setBrightness(int lightID, float bri, int type) 
{
	char command[100];
	char selector[20];
	char selectorgroup[20];
	// Normieren von 35-100 -> 1-255
	if (bri > 0) 
	{
    	bri = (bri- DIMMER_MIN )/( DIMMER_MAX - DIMMER_MIN )*254+1;
	}

	if (type==LIGHT) 
	{
   		sprintf(selector,"lights/%d/state", lightID);
		sprintf(selectorgroup,"%d", lightID);
    } else if (type==GROUP) {
   		sprintf(selector,"groups/%d/action", lightID);
		sprintf(selectorgroup,"%d", lightID);
    }

	if (bri == 0) 
	{
        sprintf(command, "{\"on\": false}");
		sleep(30);
        if (DEBUG_LEVEL > 4) printf("Light %d OFF", lightID);
    } else {
        sprintf(command, "{\"on\": true, \"bri\": %d, \"transitiontime\": %d}", (int) (bri), TRANSITION_TIME);
		sleep(30);
        if (DEBUG_LEVEL > 4) printf("Light %d ON %d%%", lightID, (int) ((bri-1)/2.55)+1);
    }
	
	sendCommand(selector, command, selectorgroup);
}

void setBrightnessAsBulb(int lightID, float bri) {
	char command[100];
	char selector[20];
	char selectorgroup[20];
	float ct,cl;
	
	// Normieren von 35-100 -> 1-255
	if (bri > 0) 
	{
    		bri = (bri- DIMMER_MIN )/( DIMMER_MAX - DIMMER_MIN )*254+1;
		cl = (DIMMER_SIMMAX - DIMMER_SIMMIN);
		ct = DIMMER_SIMMAX - (cl * log10(1+((bri-1)/254)*9));
	}

	sprintf(selector,"lights/%d/state", lightID);
	sprintf(selectorgroup,"%d", lightID);

	if (bri == 0) 
	{
        	sprintf(command, "{\"on\": false}");
			sleep(30);
        	if (DEBUG_LEVEL > 4) printf("Light %d OFF", lightID);
	} else {
		sprintf(command, "{\"on\": true, \"bri\": %d, \"ct\": %d, \"transitiontime\": %d}", (int) (bri), (int) (ct), TRANSITION_TIME);
		sleep(30);
		if (DEBUG_LEVEL > 4) printf("Light %d ON %d%% with %d", lightID, (int) ((bri-1)/2.55)+1, (int) (ct));
	}
	
	sendCommand(selector, command, selectorgroup);
}

void setCtBri(int lightID, float ctbrivalue, int type) {
	char command[100];
	char selector[20];
	char selectorgroup[20];
	float bri, ct;
	int briNorm, miredNorm;
	
	bri = floor((ctbrivalue-200000000) / 10000); // 0-100
	ct = floor((ctbrivalue-200000000) - (bri * 10000)); // Wert in Kelvin, von 2700 - 6500
	
	
	briNorm = (int) round(bri*2.55); // 0-255
	miredNorm = (int) round(1000000/ct); // Wert von 154 - 370
	
	
	
	if (type==LIGHT) 
	{
   		sprintf(selector,"lights/%d/state", lightID);
		sprintf(selectorgroup,"%d", lightID);
    } else if (type==GROUP) {
   		sprintf(selector,"groups/%d/action", lightID);
		sprintf(selectorgroup,"%d", lightID);
    }

	if (bri == 0) 
	{
        sprintf(command, "{\"on\": false}");
		sleep(30);
        if (DEBUG_LEVEL > 4) printf("Light %d OFF", lightID);
    } else {
        sprintf(command, "{\"on\": true, \"bri\": %d, \"ct\": %d, \"transitiontime\": %d}", briNorm, miredNorm, TRANSITION_TIME);
		sleep(30);
        if (DEBUG_LEVEL > 4) printf("Light %d ON %d%% %dK", lightID, (int) bri, (int) ct);
    }
	
	sendCommand(selector, command, selectorgroup);
}


void setOnOff(int lightID, float bri) 
{
	char command[100];
	char selector[20];
	char selectorgroup[20];
	
	sprintf(selector,"lights/%d/state", lightID);
	sprintf(selectorgroup,"%d", lightID);

	if (bri == 0) 
	{
        sprintf(command, "{\"on\": false}");
		sleep(30);
        if (DEBUG_LEVEL > 4) printf("Light %d OFF", lightID);
    } else {
		sprintf(command, "{\"on\": true}"); 
		sleep(30);		
        if (DEBUG_LEVEL > 4) printf("Light %d ON", lightID);
	}
	
	sendCommand(selector, command, selectorgroup);
	
}

void setColorXYB(int lightOrGroupID, float rgbvalue, int type) 
{
	char buffer[256];
	float red,green,blue;
	float cx,cy, bri;
	float X,Y,Z;
	char command[100];
	char selector[50];
	char selectorgroup[20];

    blue = floor(rgbvalue/1000000);
    green = floor((rgbvalue-blue*1000000)/1000);
	red = rgbvalue-blue*1000000-green*1000;

	bri = blue;
	if (bri < green) bri = green;
	if (bri < red) bri = red;
	bri = bri * 2.55;

	blue = blue / 100;
	green = green / 100;
	red = red / 100;

	// Apply gamma correction
	if (red > 0.04055) 
	{
		red = pow((red + 0.055) / 1.055, 2.4);
	} else {
		red = red / 12.92;
	}
	if (green > 0.04055) 
	{
		green = pow((green + 0.055) / 1.055, 2.4);
	} else {
		green = green / 12.92;
	}
	if (blue > 0.04055) 
	{
		blue = pow((blue + 0.055) / 1.055, 2.4);
	} else {
		blue = blue / 12.92;
	}
	// Convert to XYZ
	X = red * 0.649926 + green * 0.103455 + blue * 0.197109;
	Y = red * 0.234327 + green * 0.743075 + blue * 0.022598;
	Z = red * 0.013932 + green * 0.053077 + blue * 1.035763;
	// Calculate xy and bri
	if ((X+Y+Z) == 0)
	{
		cx = 0;
		cy = 0;
	} else { // round to 4 decimal max (=api max size)
		cx = X / (X + Y + Z);	
		cy = Y / (X + Y + Z);
	}

	if (type==LIGHT) 
	{
   		sprintf(selector,"lights/%d/state", lightOrGroupID);
		sprintf(selectorgroup,"%d", lightOrGroupID);
    } else if (type==GROUP) {
   		sprintf(selector,"groups/%d/action", lightOrGroupID);
		sprintf(selectorgroup,"%d", lightOrGroupID);
    }

	if (bri == 0) 
	{
        sprintf(command, "{\"on\": false}");
		sleep(30);
    } else {
		sprintf(command, "{\"xy\": [%f,%f],\"bri\": %d,\"on\":true, \"transitiontime\": %d}", cx, cy, bri, TRANSITION_TIME);
		sleep(30);
	}

	if (DEBUG_LEVEL > 4) printf(command);

	sendCommand(selector, command, selectorgroup);
}

void setColorBHS(int lightOrGroupID, float rgbvalue, int type) {
    char buffer[256];
    float red,green,blue;
    float hue,sat,bri;
    char command[100];
	char selector[50];
	char selectorgroup[20];
    
	// Hinweis: rgbvalue ist red + green*1000 + blue*1000000
    blue = floor(rgbvalue/1000000);
    green = floor((rgbvalue-blue*1000000)/1000);
	red = rgbvalue-blue*1000000-green*1000;

	// nochmal umrechnen nach hue irgendwie, weil die Living Colors Gen2 irgendwie nich gehen mit xy
	hue = 0;
	sat = 0;
	bri = 0;
	
	if (blue > 0 || green > 0 || red > 0) 
	{
	
		if ((red >= green) && (green >= blue)) 
		{
            if (red == blue) {
                 hue = 0; 
            } else {
                 hue = 60*(green-blue)/(red-blue);
            }
            sat = (red - blue) / red;
            bri = red;
		} else if ((green > red) && (red >= blue)) 
		{
			hue = 60*(2 - (red-blue)/(green-blue));
			sat = (green - blue) / green;
			bri = green;
		} else if ((green >= blue) && (blue > red)) 
		{
			hue = 60*(2 + (blue-red)/(green-red));
			sat = (green - red) / green;
			bri = green;
		} else if ((blue > green) && (green > red)) 
		{
			hue = 60*(4 - (green-red)/(blue-red));
			sat = (blue - red) / blue;
			bri = blue;
		} else if ((blue > red) && (red >= green)) 
		{
			hue = 60*(4 + (red-green)/(blue-green));
			sat = (blue - green) / blue;
			bri = blue;
		} else if ((red >= blue) && (blue > green)) 
		{
			hue = 60*(6 - (blue-green)/(red-green));
			sat = (red - green) / red;
			bri = red;
		}
		
		// Werte für HUE normieren (hue = 0-65535, sat 0-255, bri 0-255)
		hue = hue / 360 * 65535;
		sat = sat * 255;	
		bri = bri * 2.55;
	}
    
    // Ausgeben ins Log
    if (DEBUG_LEVEL > 4) printf("value:%09d, b:%d, g:%d, r: %d, hue:%d, sat: %d, bri: %d\n", rgbvalue, blue, green, red, (int)hue, (int)sat, (int)bri);
    
    if (bri == 0) 
	{
        sprintf(command, "{\"on\": false}");
		sleep(30);
        if (DEBUG_LEVEL > 4 && type==LIGHT ) printf("Light %d OFF", lightOrGroupID);
        if (DEBUG_LEVEL > 4 && type==GROUP ) printf("Group %d OFF", lightOrGroupID);
    } else {
        sprintf(command, "{\"bri\": %d, \"hue\": %d, \"sat\": %d, \"on\": true, \"transitiontime\": %d}", (int)bri, (int)hue, (int)sat, TRANSITION_TIME);
		sleep(30);
        if (DEBUG_LEVEL > 4 && type==LIGHT ) printf("Light %d ON %d%%, %d° %d%%", lightOrGroupID, (int) ((bri-1)/2.55)+1, (int) (hue / 65535 * 360), (int)(sat/2.55));
        if (DEBUG_LEVEL > 4 && type==GROUP ) printf("Group %d ON %d%%, %d° %d%%", lightOrGroupID, (int) ((bri-1)/2.55)+1, (int) (hue / 65535 * 360), (int)(sat/2.55));
    }

    if (type==LIGHT) 
	{
   		sprintf(selector,"lights/%d/state", lightOrGroupID);
		sprintf(selectorgroup,"%d", lightOrGroupID);
    } else if (type==GROUP) {
   		sprintf(selector,"groups/%d/action", lightOrGroupID);
		sprintf(selectorgroup,"%d", lightOrGroupID);
    }

   sendCommand(selector, command, selectorgroup);
    
}

void sendCommand(char* selector, char* command, char* selectorgroup) {
	
	char szTmpBuffer[128];
	int nBytesReceived = 0;
	char szBuffer[40000];
	int nCnt;
	int i = 0;
	char grouplogs[500];
	char grouplogr[500];
	
	STREAM* TcpStream = stream_create(streamname,0,0);
	if (TcpStream == NULL) 
	{
        printf("Creating Stream failed");
        stream_close(TcpStream);
		return;
    }

    char buffer[1024];
	// send Command to Hue
    sprintf(buffer, "PUT /api/%s/%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n%s", 
    		USERNAME, selector, IP_ADDRESS, strlen(command), command);
    if (DEBUG_LEVEL > 1) printf(buffer);
    stream_write(TcpStream,buffer,sizeof(buffer));
	sleep(30);
	if (DEBUG_LEVEL > 2)
	{
		sprintf(grouplogs,"/log/hue_%s_s.log", selectorgroup);
		if (DEBUG_LEVEL > 2) printf("File %s",grouplogs);
		FILE* ps = fopen(grouplogs,"w"); // open file
		fprintf(ps,"%s",buffer); // write file
		fclose(ps);	
	}
    stream_flush(TcpStream);
	
	if (DEBUG_LEVEL > 2)
	{
		// read stream from Hue
		do
		{
			nCnt = stream_read(TcpStream,szTmpBuffer,128,4000);
			if (nCnt + nBytesReceived > 40000)
			{
				 nBytesReceived = -1;
				 break; //File is too large
			}
			 else if(nCnt > 0)
			{
				 strncpy((char*)szBuffer + nBytesReceived, szTmpBuffer, nCnt);
				 nBytesReceived += nCnt;
			}
		 } 
		while (nCnt > 0);
			if (nBytesReceived > 0)
			{
				sprintf(grouplogr,"/log/hue_%s_r.log", selectorgroup);
				if (DEBUG_LEVEL > 2) printf("File %s",grouplogr);
				FILE* pr = fopen(grouplogr,"w"); // open file
				fprintf(pr,"%s",szBuffer); // write file
				fclose(pr);
		} 
	}
    stream_close(TcpStream);
}

printf("Program loop Started");
while (1==1) 
{
    nEvents = getinputevent();
	
	// Sonderfunkion: AI12 löst ein "ALLES AUSSCHALTEN"-Signal an die HUE-Bridge aus
	if (nEvents & 0x8 << 12 && getinput(12)==1) 
	{
		if (DEBUG_LEVEL > 4) printf("All Lights OFF");
		sendCommand( "groups/0/action", "{\"on\":false}");
		// Alle anderen Eingänge werden an die HUE laut Konfiguration weitergegeben:
	} else {
		int i;
		for (i = 0; i < 12; i++) 
		{
			if (nEvents & 0x8 << i) 
			{
				updateLamp(i, getinput(i));
				sleep(150);
			}
		}
	}
	// Slow down the program
    sleep(100);
}
