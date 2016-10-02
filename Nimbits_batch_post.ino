
/*
current on October-03-2012. StewieT.

batch posts to nimbits
Uses the PString.h library.
Instead of making many connections to the server using the usual Arduino ethernet functions
like 'client.print' (every statement is a separate set of ethernet transactions), all the data to send in the 
POST trasaction is 'assembled' into one string using the Pstring library, which then gets sent in one hit.
This reduces the IP traffic considerably and when faultfinding with a packet sniffer, its easier
to see what you are looking for

This example is tested on an Arduino MEGA (a freetronics ethermega) so remember to adjust your pins in the setup() section
if using with a different board type
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PString.h>            // from http://arduiniana.org/libraries/pstring/
                                // allows 'printing' to a string buffer


char buffer[400];            // larger than required for this example, resize to suit your application
char content[200];           // larger than required for this example, resize to suit your application
PString str(buffer, sizeof(buffer));
PString cont(content, sizeof(content));

int ana_A ;    // variables to store analog samples
int ana_B ;


/************ ETHERNET STUFF ************/
byte mac[] = { 
    0xDE, 0xAD, 0x45, 0xEF, 0xFE, 0xED };

// adjust these to suit your local setup. 
byte ip[] = {192,168,10,234 };                // ethernet cards address
byte nameserver[] = {192,168,10,1 };
byte gateway[] = {192,168,10,1 };
byte subnet[] = {255,255,255,0 };

EthernetClient nameofclient;
//***************************************

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 120000;  // 120-sec delay between updates to logging service, 720 per 24hrs to keep within the 1000 quota
                                               // and leave room for other types of API access 

// these are for your nimbits account
char mailaddr [] = "XXXYYYZZZ@yahaa.co.uk";    // your email address for the logging account
char key [] = "AAABBBCCC";                     // your KEY you created for the account. Not the UUID.


void setup() {

    pinMode(53, OUTPUT);    // set the hardware SS pin as an output (necessary!) pin 53 on mega. This pin is not actualy used for SS.
    
    pinMode(10,OUTPUT);     // Pin 10 is what is used on the freetronics ethermega so set the pin to OUTPUT
    digitalWrite(10, HIGH); // and start off with the W5100 ethernet chip de-selected! This is because I also use the SD card (not in this sketch)

    Serial.begin(57600);    // serial debug

    Ethernet.begin(mac, ip, nameserver, gateway, subnet);            // initialise the ethernet system

    Serial.println("up and running....");
    Serial.print("POST interval is ");
    Serial.print(postingInterval/1000);
    Serial.println(" seconds");
    
}


// =========================================================================
// very simple main loop. Just constantly reads two ADC channels and then if its time to log again, do_weblog() sends in the latest values
// of A8 and A9
void loop()
{

    ana_A = analogRead(A8);    // read some voltage and save it
    ana_B = analogRead(A9);    // read some voltage and save it

    do_weblog();

} 

// =========================================================================


/*
This is where it all happens.
If the posting interval is reached then a new POST is done with the latest data
If not time yet, we simply exit
 */
void do_weblog() {

    // if you're not connected, and 'postinginterval' secs have passed since
    // your last connection, then connect again and send data:
    if(!nameofclient.connected() && (millis() - lastConnectionTime > postingInterval)) {
        str.begin();    // reset the into-string pointer. This is the 'final' composite string being assembled.
        cont.begin();   // and for the actual content ie payload string

        sendData();    // the POST gets created and sent here

        delay(1);
        nameofclient.stop();    // stop the client
        nameofclient.flush();   // and tidy up
    }
}



void sendData() {

    // Create the 'content' string to send. Its assembled from the user details, then the sensor data
    // 1st part of content is access-details
    cont.print("email=");
    cont.print(mailaddr);
    cont.print("&key=");
    cont.print(key);

    // next get the data and store it in the 'cont' string
    // The data is saved into the data-points 'test123' & 'test456'. These are created in the control console
    // web page (currently http://www.cloud.nimbits.com )
    cont.print("&p1=test123&v1=");        // test123 is the name of a data point you created
    cont.print(ana_A,DEC);                // latest analog value. Replace this with whatever you are logging
    cont.print("&p2=test456&v2=");        // test456 is another data point you created
    cont.print(ana_B,DEC);                // latest analog value. Replace this with whatever you are logging
                                          // This example only shows two points. It can be extended up to 100
                                          // but remember to adjust the buffer sizes where the strings get assembled

    // now get the length of the assembled content string. This string is the complete 'content' and includes
    // email addr, access key, sensor data (two analog vales in this case)
    int contlen = (cont.length());

    // now try and connect to the web-site
    Serial.println("connecting...");
    if (nameofclient.connect("cloud.nimbits.com", 80))     // make the attempt...
    { 
        // the format of the POST section below seems to be fairly critical. 
        str.print("POST /service/batch HTTP/1.1\r\n");
        str.print("Host: cloud.nimbits.com\r\n");
        str.print("Connection: close\r\n");
        str.print("Cache-Control: max-age=0\r\n");
        str.print("Content-Length: ");
        str.print(contlen,DEC);
        str.print("\r\n");
        str.print("Content-Type: application/x-www-form-urlencoded\r\n");
        str.print("\r\n");  // this empty line is REQUIRED
        str.print(cont);    // the actual content string 'cont' (access details, data points)
        str.print("\r\n");  // and a terminating newline
// this completes the assembly of the string to send (contained in 'str')

// the total string (post headers and content) is now sent to the ethernet connection in one hit
        nameofclient.print(str);  // ethernet send to COSM

        Serial.println();        // for debug
        
        Serial.print(str);       // this is a copy of whats sent to the ethernet (the same string)
                                 // you can comment-out the line: nameofclient.print(str) above to just see the debug output
                                 // while testing. This lets you see if your POST looks sensible.
        
        Serial.println();        // for debug
        Serial.println();        // for debug


// shows the response from the server. Uncomment below if required.
//        delay(1500);        // wait 1.5 sec for a response
//        while (nameofclient.available()) {
//          char c = nameofclient.read();
//          Serial.print(c);
//        }


    }

    else {
        // if you couldn't make a connection:
        Serial.println();
        Serial.println("Connection failed");
        Serial.println("disconnecting.");
        Serial.println();
    }
    // note the time that the connection was made
    lastConnectionTime = millis();
}


