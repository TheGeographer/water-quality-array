// Uno_I2C_Collector
// 7/16/2015 Eric Compas, ericcompas@gmail.com
//
// based on I2C example provided by Atlas Scientific
// written for Arduino Uno
// designed to work with Atlas Scientific EZO series of controllers

// Control script to recieve incoming commands from USB and/or Bluetooth and pass to appropropriate probe.
// 1:ph
// 2:DO
// 3:EC
// 4:reserved
// 5:temp
// 0:(special) -- read all

// command takes the form of CHANNEL:COMMAND, for example 1:r would return a reading from the pH probe

// import libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>               
#include <SoftwareSerial.h>

// define constants/pins
#define PH_I2C 90               // I2C ID number for EZO pH Circuit
#define DO_I2C 91               // I2C ID number for EZO DO Circuit
#define EC_I2C 92               // I2C ID number for EZO EC Circuit
#define ONE_WIRE_BUS 7         // temp sensor pin
#define BT_RX 5                // bluetooth RX pin
#define BT_TX 6               // bluetooth TX pin

// global variables
char computerdata[20];              //A 20 byte character array to hold incoming data from a pc/mac/other
char btdata[20];                    //a 20 byte character array to hold incoming data from blutooth
char sensordata[30];                //A 30 byte character array to hold incoming data from the sensors
char returndata[100];
byte computer_bytes_received=0;     // number of characters bytes have been received on I2C channel
byte bt_bytes_received=0;           // number of bluetooth bytes received
byte sensor_bytes_received=0;       //We need to know how many characters bytes have been received
byte code=0;                        //used to hold the I2C response code. 
byte trans=0;
byte in_char=0;                  //used as a 1 byte buffer to store in bound bytes from the pH Circuit.   
byte i=0;                        //counter used for ph_data array. 
byte readallflag=0;              // flag for reading all
int time=1100;                      //used to change the delay needed depending on the command sent to the EZO Class pH Circuit.
int address;

// temperature compensation -- PLEASE CHECK BEFORE COMPILING
//float tempComp = 0.21;            // temperature compensation for Unit #2
//float tempComp = 0.0;            // temperature compensation for Blue unit (RRC)
float tempComp = 0.5;            // temperature compensation for Red unit (Current Unit #1)
float tempFloat;

char *channel;                      //Char pointer used in string parsing
char *cmd;                          //Char pointer used in string parsing

char *do_mgl;          // DO as mg/L
char *do_perc;         // DO as percent saturation
char *ec_us;           // EC as uS
char *ec_tds;          // EC as total dissolved solids
char temp[6];          // temp in C

// setup temp
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// setup bluetooth softserial
SoftwareSerial btSerial(BT_RX, BT_TX); // rx,tx

void setup() {
  // start serial port to serial monitor/USB
  Serial.begin(9600);
  Serial.println("Starting I2C multiport on Uno Tentacle...");
  Serial.println("Version: 1.1 March 31, 2016");
  
  // start temp WIRE library
  Wire.begin();
 
  // start bluetooth softserial port 
  btSerial.begin(9600);
  //btSerial.println("Starting I2C multiport on Nano...");
  
  // Start up the temp sensor
  sensors.begin();
  Serial.println("Setup complete");
  }

// serial port interrupt -- from USB/Serial monitor on pc 
void serialEvent(){   
  computer_bytes_received=Serial.readBytesUntil(13,computerdata,20); //read until <CR> and count characters received    
  computerdata[computer_bytes_received]=0; //add a 0 after the last character we received
}    
       
void loop(){
  // check bluetooth soft serial port
  int bt = 0;
  while (btSerial.available()) {
    // set data as if received from USB (hard) serial port)
    char c = btSerial.read();
    Serial.print("Char: ");
    Serial.println(c);
    computerdata[computer_bytes_received] = c;
    computer_bytes_received += 1;     
    bt = 1;
  }  
  if (bt) {
    computerdata[computer_bytes_received]=0;
    Serial.print("Bluetooth command received: ");
    Serial.println(computerdata);
  }
  
  // check USB (hard) serial port  
  if(computer_bytes_received!=0){                 //If computer_bytes_received does not equal zero  
    // parse incoming channel and command
    channel=strtok(computerdata, ":");            //Let's pars the string at each colon
    cmd=strtok(NULL, ":");             //Let's pars the string at each colon 
    Serial.print("Channel: ");
    Serial.println(channel);
    Serial.print("Command: ");
    Serial.println(cmd);
    
    // handle special channel "0" commands
    if (strcmp(channel,"0")==0){
      readallflag = 1;
      switch(*cmd){
        case 'r':
          read_all();        // take reading from all probes
          break;
        case 'c':           // set temp compensation for all probes (for calibration)
          set_temp_all();
          break;
        case 's':            // set up all probes
          set_all();
          break;
        case 'i':            // read info from all probes
          info_all();
          break;
        case 't':            // read the temp probe (not an I2C probe)
          read_temp();
          break;
      }
    } else {
      readallflag = 0;
      // regular command -- open I2C port and send command
      open_channel();
      i2c_cmd();
    }
    computer_bytes_received=0;       // reset command received count
  }
}

// send and receive I2C command
void i2c_cmd(){
  // set wait time on response
  if(cmd[0]=='c'||cmd[0]=='C')time=2000;          // if calibrating, wait longest (EC requires 2 seconds for 'dry' calibration)
    else if (cmd[0]=='r'||cmd[0]=='R')time=1400;  // if reading, wait long  
    else time=300;                                //any other command, wait only 300ms
  Serial.print("I2C command: ");
  Serial.println(cmd);
  Serial.print("I2C wait time: ");
  Serial.println(time);
  
  // send command
  Wire.beginTransmission(address); //call the circuit by its ID number.  
  Wire.write(cmd);        //transmit the command that was sent through the serial port.
  trans=Wire.endTransmission();          //end the I2C data transmission. 
  Serial.print("Wire.endTransmission return code: ");
  Serial.println(trans);
  
  delay(time);                    //wait the correct amount of time for the circuit to complete its instruction. 
  Wire.requestFrom(address,20,1); //call the circuit and request 20 bytes (this may be more then we need).
  code=Wire.read();        //the first byte is the response code, we read this separately.  
  Serial.print("Wire.read code: ");
  Serial.println(code);
  
  
  // get result
  while(Wire.available()){          //are there bytes to receive.  
   in_char = Wire.read();           //receive a byte.
   sensordata[i]= in_char;             //load this byte into our array.
   i+=1;                            //incur the counter for the array element. 
    if(in_char==0){                 //if we see that we have been sent a null command. 
        i=0;                        //reset the counter i to 0.
        Wire.endTransmission();     //end the I2C data transmission.
        break;                      //exit the while loop.
    }
  }
  Serial.print("I2C Received: ");
  Serial.println(sensordata);          //print the data.  
  
  if (readallflag==0){
    // report result to bluetooth only if not read all command
    if (code==1){
      btSerial.print("Success: ");
    } else {
      btSerial.print("Failed: ");
    }
    btSerial.println(sensordata);       // send response to bluetooth if not "read all" option
  }
  computer_bytes_received=0;                   //reset the serial event flag.
}

void open_channel(){                                  //This function controls what I2C port is opened. 
  switch(*channel){
    case '1':                        // ph probe channel
        address = PH_I2C;
      break;
    case '2':                        // do probe channel
      address = DO_I2C;
      break;
    case '3':                        // ec probe channel
      address = EC_I2C;
      break;
    case '4':                        // reserved
      address = 0;
      break; 
 }
}

// initialize all probes to default settings
void set_all(){
  // pH settings
  Serial.println("Initializing pH...");
  channel = "1";
  open_channel();  
  //Serial1.print("l1\r");    // turn on LEDs
  //Serial1.print("e\r");     // enter standby mode
  
  // DO settings
  Serial.println("Initializing DO...");
  channel = "2";
  open_channel();
  cmd = "o,%,1";
  i2c_cmd();
 
  // EC settings
  Serial.println("Initializing EC...");
  channel = "3";
  open_channel(); 
  delay(50); 
  //Serial1.print("l,1\r");          // turn on LEDs
  delay(50);
  //Serial1.print("c,0\r");           // enter standby mode
  delay(50);
  //Serial1.print("response,0\r");  // disable response codes
  //Serial1.print("o,s,0\r");       // disable salinity reporting
  //Serial1.print("o,sg,0\r");      // disable specific gravity reporting
}

// return info from all probes
void info_all(){
  Serial.print("info_all() not implemented yet");
}
  
  
void read_all(){                                      // read values from all probes and return aggregated results   
  // clear data string
  returndata[0] = (char)0;
  
  // read temp
  Serial.println("Reading temp...");
  sensors.requestTemperatures();
  Serial.print("Temp: ");
  Serial.println(sensors.getTempCByIndex(0));
  dtostrf(sensors.getTempCByIndex(0),5,2,temp);   // make sure sufficient space, 5 char
  strcat(returndata,"temp_c:");
  strcat(returndata,temp);
  
  // read pH
  Serial.println("Reading pH...");
  channel = "1";
  open_channel();
  char test[20] = "t,";
  Serial.println("Sending temp to pH probe...");
  strcat(test,temp);
  Serial.print("Temp cmd: ");
  Serial.println(test);
  cmd = test;
  i2c_cmd();
  delay(300);
  cmd = "r";
  i2c_cmd();
  strcat(returndata,";ph:");
  strcat(returndata,sensordata);
  Serial.print("Returndata: ");
  Serial.println(returndata);
  
  // read DO
  Serial.println("Reading DO...");
  channel = "2";
  open_channel();
  Serial.println("Sending temp to DO probe...");
  cmd = test;
  i2c_cmd();
  delay(300);
  cmd = "r";
  i2c_cmd();
  if((sensordata[0] >= 48) && (sensordata[0] <=57)){   //if DO_data[0] is a digit and not a letter
    do_mgl=strtok(sensordata, ",");    // parse the percent saturation value 
    do_perc=strtok(NULL, ",");        // parse the mg/L value
  } else {
    do_perc="-1";  // error with reading, return -1
    do_mgl="-1";
  } 
  strcat(returndata,";do_mgl:");
  strcat(returndata,do_mgl);
  strcat(returndata,";do_perc:");
  strcat(returndata,do_perc);
  Serial.print("Returndata: ");
  Serial.println(returndata);
  
  // read EC
  Serial.println("Reading EC...");
  channel = "3";
  open_channel();
  Serial.println("Sending temp to EC probe...");
  cmd = test;
  i2c_cmd();
  delay(300);
  cmd = "r";
  i2c_cmd();
  if((sensordata[0] >= 48) && (sensordata[0] <=57)){   //if DO_data[0] is a digit and not a letter
    ec_us=strtok(sensordata, ",");    // parse the percent saturation value 
    ec_tds=strtok(NULL, ",");        // parse the mg/L value
  } else {
    ec_us="-1";  // error with reading, return -1
    ec_tds="-1";
  } 
  strcat(returndata,";ec_us:");
  strcat(returndata,ec_us);
  strcat(returndata,";ec_tds:");
  strcat(returndata,ec_tds);
  Serial.print("Returndata: ");
  Serial.println(returndata);
  //btSerial.print("Data: ");
  btSerial.println(returndata);
}

void set_temp_all(){                                      // read values from all probes and return aggregated results   
  Serial.println("Set temp compensation for all probes...");
  
  // read temp
  Serial.println("Reading temp...");
  sensors.requestTemperatures();
  Serial.print("Temp: ");
  Serial.println(sensors.getTempCByIndex(0));
  tempFloat = sensors.getTempCByIndex(0);
  tempFloat = tempFloat + tempComp;
  Serial.print("Temp with comp: ");
  Serial.println(tempFloat);
  //dtostrf(sensors.getTempCByIndex(0),5,2,temp);   // make sure sufficient space, 5 char
  dtostrf(tempFloat,5,2,temp);   
  
  // set pH temp
  channel = "1";
  open_channel();
  char test[20] = "t,";
  Serial.println("Sending temp to pH probe...");
  strcat(test,temp);
  Serial.print("Temp cmd: ");
  Serial.println(test);
  cmd = test;
  i2c_cmd();
  
  // set DO temp
  channel = "2";
  open_channel();
  Serial.println("Sending temp to DO probe...");
  cmd = test;
  i2c_cmd();
  
  // set EC temp
  channel = "3";
  open_channel();
  Serial.println("Sending temp to EC probe...");
  cmd = test;
  i2c_cmd();
}
  
void read_temp(){
  // read temp
  Serial.println("Reading temp...");
  sensors.requestTemperatures();
  Serial.print("Temp: ");
  Serial.println(sensors.getTempCByIndex(0));
  tempFloat = sensors.getTempCByIndex(0);
  tempFloat = tempFloat + tempComp;
  Serial.print("Temp with comp: ");
  Serial.print(tempFloat);
  //Serial2.println(sensors.getTempCByIndex(0));
  //dtostrf(sensors.getTempCByIndex(0),5,2,temp);   // make sure sufficient space, 5 char
  //strcat(returndata,"temp:");
  //strcat(returndata,temp);
}
  



