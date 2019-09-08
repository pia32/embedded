#include "SparkFun_Ublox_Arduino_Library.h"
SFE_UBLOX_GPS myGPS;

uint32_t GPSLastUpdate = 0;
int32_t GPSLat;
int32_t GPSLon;
float GPSAlt;
int32_t GPSFixType;
int32_t GPSVelN, GPSVelE, GPSVelD;
int32_t GPSHeading;
int32_t GPSVAcc;
int32_t GPSHAcc;

void GPSInit() {
  Serial1.begin(19200); //Ublox RTK
  delay(500);

  while(myGPS.begin(Serial1) == false)
    delay(10);
  
  while(myGPS.setAutoPVT(true) != true) //Tell the GPS to "send" each solution
    delay(10);

  while(myGPS.setNavigationFrequency(8) != true) //8hz results in fastest updates. I think 19200 baud rate is clogging up
    delay(10);

  delay(500);
}

void GPSPoll() {
  if (!myGPS.getPVT())
    return;
    
  GPSLat = myGPS.getLatitude();
  GPSLon = myGPS.getLongitude();
  GPSAlt = myGPS.getAltitude() / 1000.0;
  GPSFixType = myGPS.getFixType() + (myGPS.getCarrierSolutionType() * 10);//1x for floatRTK, 2x for fixedRTK
  GPSVelN = myGPS.velN;
  GPSVelE = myGPS.velE;
  GPSVelD = myGPS.velD;
  GPSHeading = myGPS.headingOfMotion;
  GPSVAcc = myGPS.vAcc;
  GPSHAcc = myGPS.hAcc;
    
  GPSLastUpdate = millis();
}
