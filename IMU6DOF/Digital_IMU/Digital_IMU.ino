#include <Wire.h>

#define gyroAddress 0x68
#define adxlAddress 0x53

double zeroValue[5] = {-200, 44, 660, -18.5, 52.3}; // Found by experimenting

/* All the angles start at 180 degrees */
double gyroXangle = 180;
double gyroYangle = 180;

double compAngleX = 180;
double compAngleY = 180;

unsigned long timer;

void setup() {  
  Serial.begin(115200);
  Wire.begin();
  
  i2cWrite(adxlAddress,0x31,0x09); // Full resolution mode
  i2cWrite(adxlAddress,0x2D,0x08); // Setup ADXL345 for constant measurement mode
  
  i2cWrite(gyroAddress,0x16,0x1A); // this puts your gyro at +-2000deg/sec  and 98Hz Low pass filter
  i2cWrite(gyroAddress,0x15,0x09); // this sets your gyro at 100Hz sample rate

  timer = micros();
}

void loop() {
  double gyroXrate = (((double)readGyroX()-zeroValue[3])/14.375);
  gyroXangle += gyroXrate*((double)(micros()-timer)/1000000); // Without any filter
  
  double gyroYrate = -(((double)readGyroY()-zeroValue[4])/14.375);
  gyroYangle += gyroYrate*((double)(micros()-timer)/1000000); // Without any filter
  
  double accXangle = getXangle();
  double accYangle = getYangle();
  
  compAngleX = (0.93*(compAngleX+(gyroXrate*(double)(micros()-timer)/1000000)))+(0.07*accXangle);
  compAngleY = (0.93*(compAngleY+(gyroYrate*(double)(micros()-timer)/1000000)))+(0.07*accYangle);
  
  double xAngle = kalmanX(accXangle, gyroXrate, (double)(micros() - timer)); // calculate the angle using a Kalman filter
  double yAngle = kalmanY(accYangle, gyroYrate, (double)(micros() - timer)); // calculate the angle using a Kalman filter
  
  timer = micros();
  
  /* print data to processing */
  Serial.print(gyroXangle);Serial.print("\t");
  Serial.print(gyroYangle);Serial.print("\t");
  
  Serial.print(accXangle);Serial.print("\t");
  Serial.print(accYangle);Serial.print("\t");
  
  Serial.print(compAngleX);Serial.print("\t");  
  Serial.print(compAngleY); Serial.print("\t"); 
  
  Serial.print(xAngle);Serial.print("\t");
  Serial.print(yAngle);Serial.print("\t");
   
  Serial.print("\n");
  
  delay(10);
}
void i2cWrite(uint8_t address, uint8_t registerAddress, uint8_t data){
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.write(data);
  Wire.endTransmission();
}
uint8_t i2cRead(int address, uint8_t registerAddress) {
  uint8_t data;  
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.endTransmission();  
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 1);
  data = Wire.read();
  Wire.endTransmission();  
  return data;
}
int readGyroX(void) {
  int data;
  data = i2cRead(gyroAddress, 0x1D)<<8;
  data |= i2cRead(gyroAddress,0x1E);      
  return data;
}
int readGyroY(void) {
  int data;
  data = i2cRead(gyroAddress, 0x1F)<<8;
  data |= i2cRead(gyroAddress,0x20);      
  return data;
}
double getXangle() {
  double accXval = (double)readAccX()-zeroValue[0];
  double accZval = (double)readAccZ()-zeroValue[2];
  double angle = (atan2(accXval,accZval)+PI)*RAD_TO_DEG;
  return angle;
}
double getYangle() {
  double accYval = (double)readAccY()-zeroValue[1];
  double accZval = (double)readAccZ()-zeroValue[2];
  double angle = (atan2(accYval,accZval)+PI)*RAD_TO_DEG;
  return angle;
}
int readAccX(void) {
  int data;
  data = i2cRead(adxlAddress, 0x32);
  data |= i2cRead(adxlAddress,0x33)<<8;      
  return data;
}
int readAccY(void) {
  int data;
  data = i2cRead(adxlAddress, 0x34);
  data |= i2cRead(adxlAddress,0x35)<<8;      
  return data;
}
int readAccZ(void) {
  int data;
  data = i2cRead(adxlAddress, 0x36);
  data |= i2cRead(adxlAddress,0x37)<<8;      
  return data;
}