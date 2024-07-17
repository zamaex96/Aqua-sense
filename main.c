#include <Arduino_HTS221.h> //Include library to read Temperature and Humidity
#include <algorithm>
#define HIGH 1
#define LOW 0
#define size_of_buffer 8
#define BITLENGTH 8 //1 Byte
//#define FIRSTBIT pow(2,(BITLENGTH-1))//MSB Value
#define FIRSTBIT (1 << (BITLENGTH - 1))
#define TRUE 1
#define sensorPin1 A0
#define sensorPin2 A2
#define sensorPin3 A4
#define LED 4
#define HB 255
#define SB 180
#define EP 255
#define EB 00

uint16_t v_R1=0;
uint16_t v_R2=0;
uint16_t v_R3=0;
uint16_t v_R=0;
uint16_t rawValues[3];
uint8_t token=0;
uint16_t Th_voltages[8];
uint16_t Th_voltage=0;
uint16_t Delay=500;
uint8_t CT_ID_EGC=0;
uint8_t CT_ID_MLC=0;
uint8_t CT_ID_SC=0;
uint8_t CT_ID=0;
uint8_t RecPacket=0;
uint16_t Avg=0;
uint8_t countB1=0;
uint8_t countB0=0;
int input=0;
int cntR1_1=0,cntR2_1=0,cntR3_1=0;
int cntR1_0=0,cntR2_0=0,cntR3_0=0;
int BestRx=0; int bit_data[4]; char SyncBytes[2];uint8_t syncByte;

void DiversityGainEngineInit();
uint16_t ThresholdVoltageEstimator(uint16_t v1, uint16_t v2, uint16_t v3);
void CombiningTEchniqueSelection();
uint8_t EGC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t tkn,uint8_t num);
uint8_t MLC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t tkn,uint8_t num);
uint8_t SC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t num);
uint8_t SC_EngineReception(uint16_t voltage,uint8_t num);int ComMode=0;int integer=0;int diff_rec=0;

float Pressure;
float Temperature[100], Humidity;
float OldTemp, NewTemp, DiffTemp;
//int c = 0;
int chk=0;
 float low=-4.0, high=4.0;

void setup() {
Serial.begin(115200);

 pinMode(LED,OUTPUT);
 HTS.begin();  
  if (!HTS.begin()) //Initialize Temperature and Humidity sensor
  { Serial.println("Failed to initialize Temperature and Humidity Sensor!"); while (1);}
}

void loop() {

DiversityGainEngineInit(); //for receiving packets
// BEE();  // for transmitting the dataframe 

}

void BEE()
{
int count=0;
OldTemp = HTS.readTemperature();
NewTemp = HTS.readTemperature();
DiffTemp = OldTemp- NewTemp;
++count;
 writeByte(EP);writeByte(EP);writeByte(HB);writeByte(SB); writeByte(NewTemp);writeByte(EB);Serial.println(NewTemp);
if ((chk=inRange(low,high,DiffTemp))== 1)
        {
         // writeByte(HB);writeByte(SB); 
         digitalWrite(LED,HIGH);delayMicroseconds(1000);
         PulseMapping(DiffTemp);
         writeByte(EB);
         Serial.println(DiffTemp);
          }
 else 
 {
 // writeByte(HB);writeByte(SB); 
  digitalWrite(LED,LOW);delayMicroseconds(1000); 
  writeByte(NewTemp);
  writeByte(EB);Serial.println(NewTemp);
  }
  
 }


void DiversityGainEngineInit()
{
  for(int j=0;j<BITLENGTH;j++)
  {
     v_R1=analogRead(sensorPin1);//Read the sensor1 Value
     v_R2=analogRead(sensorPin2);//Read the sensor1 Value
    v_R3=analogRead(sensorPin3);//Read the sensor1 Value
  
  Th_voltages[j]=ThresholdVoltageEstimator(v_R1,v_R2,v_R3);
  delayMicroseconds(1000);
  }
  Th_voltage=(Th_voltages[0]+Th_voltages[1]+Th_voltages[2]+Th_voltages[3]+Th_voltages[4]+Th_voltages[5]+Th_voltages[6]+Th_voltages[7])/8;
  CombiningTEchniqueSelection();
}

uint16_t  ThresholdVoltageEstimator (uint16_t v1, uint16_t v2, uint16_t v3)
{
 return std::max({v1, v2, v3});
}

void CombiningTEchniqueSelection() {
  for (uint8_t j = 0; j < BITLENGTH; j++) {
    v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
    CT_ID_EGC = EGC_Engine(v_R1, v_R2, v_R3, HIGH, j);
    CT_ID_MLC = MLC_Engine(v_R1, v_R2, v_R3, HIGH, j);
    CT_ID_SC = SC_Engine(v_R1, v_R2, v_R3, j);
    delayMicroseconds(1000);
  }

  countB1 = 0; cntR1_1;
  if (CT_ID_EGC > CT_ID_MLC && CT_ID_EGC > CT_ID_SC) { EGC_machine();} 
  else if (CT_ID_MLC > CT_ID_EGC && CT_ID_MLC > CT_ID_SC) {MLC_machine();}
  else if (CT_ID_SC > CT_ID_EGC && CT_ID_SC > CT_ID_MLC) {SC_machine();}
}


void EGC_machine()
{   
    for (uint8_t k = 0; k < 2; k++) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
     v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
     delayMicroseconds(1000);
    syncByte=EGC_Engine(v_R1, v_R2, v_R3, LOW, j);
      SyncBytes[k]=syncByte}}
    if (SyncBytes[0] == HB && SyncBytes[1]==SB) {
    v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
    delayMicroseconds(1000);
    ComMode = EGC_Engine_1bit(v_R1, v_R2, v_R3);
    if (ComMode == 0) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
        uint8_t RecPacket = EGC_Engine(v_R1, v_R2, v_R3, LOW, j);}
        input = 0;
        writeByte(EP);
        writeByte(EP); 
       writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket); // Transmit one byte
        writeByte(EB);
      
    } 
    else if (ComMode == 1) {
      for (uint8_t j = 0; j < 4; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
        delayMicroseconds(1000);
        bit_data[j] = EGC_Engine_1bit(v_R1, v_R2, v_R3);
      }
      if (bit_data[0] == 1 && bit_data[1] == 0) {
        integer = +1;
      } else if (bit_data[0] == 0 && bit_data[1] == 1) {
        integer = -1;
      }
      if (bit_data[2] == 1 && bit_data[3] == 1) {
        diff_rec = 1;
      }
      if (bit_data[2] == 0 && bit_data[3] == 0) {
        diff_rec = 2;
      }
      if (bit_data[2] == 1 && bit_data[3] == 0) {
        diff_rec = 3;
      }
      if (bit_data[2] == 0 && bit_data[3] == 1) {
        diff_rec = 4;
      }
      if (integer == +1) {
       writeByte(EP);
       writeByte(EP);
       writeByte(HB);
       writeByte(SB);
        writeByte(RecPacket - diff_rec); // Transmit one byte
        writeByte(EB);
      } else if (integer == -1) {
        writeByte(EP);
        writeByte(EP); 
       writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket + diff_rec); // Transmit one byte
        writeByte(EB);
      }
    }}
}

void MLC_machine()
{    
    for (uint8_t k = 0; k < 2; k++) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
     v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
     delayMicroseconds(1000);
    syncByte=MLC_Engine(v_R1, v_R2, v_R3, LOW, j);
      SyncBytes[k]=syncByte}}
    if (SyncBytes[0] == HB && SyncBytes[1]==SB) {
    v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
     delayMicroseconds(1000);
    ComMode = MLC_Engine_1bit(v_R1, v_R2, v_R3);
    if (ComMode == 0) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
        uint8_t RecPacket = MLC_Engine(v_R1, v_R2, v_R3, LOW, j);}
        input = 0;
       writeByte(EP);
       writeByte(EP); 
       writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket); // Transmit one byte
        writeByte(EB);
      
    } else if (ComMode == 1) {
      for (uint8_t j = 0; j < 4; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
         delayMicroseconds(1000);
        bit_data[j] = MLC_Engine_1bit(v_R1, v_R2, v_R3);
      }
      if (bit_data[0] == 1 && bit_data[1] == 0) {
        integer = +1;
      } else if (bit_data[0] == 0 && bit_data[1] == 1) {
        integer = -1;
      }
      if (bit_data[2] == 1 && bit_data[3] == 1) {
        diff_rec = 1;
      }
      if (bit_data[2] == 0 && bit_data[3] == 0) {
        diff_rec = 2;
      }
      if (bit_data[2] == 1 && bit_data[3] == 0) {
        diff_rec = 3;
      }
      if (bit_data[2] == 0 && bit_data[3] == 1) {
        diff_rec = 4;
      }
      if (integer == +1) {
        writeByte(EP);
        writeByte(EP);
        writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket - diff_rec); // Transmit one byte
        writeByte(EB);
      } else if (integer == -1) {
        writeByte(EP);
        writeByte(EP);
        writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket + diff_rec); // Transmit one byte
        writeByte(EB);
      }
    } }
}

void SC_machine(){
      for (uint8_t k = 0; k < 2; k++) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
     v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
     delayMicroseconds(1000);
     v_R=best_SC_Rx(v_R1, v_R2, v_R3);
    syncByte=SC_EngineReception(v_R, j);
      SyncBytes[k]=syncByte}}
    if (SyncBytes[0] == HB && SyncBytes[1]==SB) {
    v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
    v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
    v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
    delayMicroseconds(1000);
    v_R=best_SC_Rx(v_R1, v_R2, v_R3);
    ComMode = SC_Engine_1bit(v_R);
    if (ComMode == 0) {
      for (uint8_t j = 0; j < BITLENGTH; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
        delayMicroseconds(1000);
        v_R=best_SC_Rx(v_R1, v_R2, v_R3);
        uint8_t RecPacket = SC_EngineReception(v_R, j);}
        input = 0;
         writeByte(EP);
        writeByte(EP);
        writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket); // Transmit one byte
        writeByte(EB);
      
    }
    if (ComMode == 1) {
      for (uint8_t j = 0; j < 4; j++) {
        v_R1 = analogRead(sensorPin1); // Read the sensor1 Value
        v_R2 = analogRead(sensorPin2); // Read the sensor2 Value
        v_R3 = analogRead(sensorPin3); // Read the sensor3 Value
       delayMicroseconds(1000);
       v_R=best_SC_Rx(v_R1, v_R2, v_R3);
        bit_data[j] = SC_Engine_1bit(v_R);
      }
      if (bit_data[0] == 1 && bit_data[1] == 0) {
        integer = +1;
        } else if (bit_data[0] == 0 && bit_data[1] == 1) {
         integer = -1;
         }

     switch ((bit_data[2] << 1) | bit_data[3]) {
         case 0b11:
        diff_rec = 1;
        break;
       case 0b00:
        diff_rec = 2;
        break;
        case 0b10:
        diff_rec = 3;
        break;
         case 0b01:
        diff_rec = 4;
        break;
        }
      if (integer == +1) {
       writeByte(EP);
        writeByte(EP); 
       writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket - diff_rec); // Transmit one byte
        writeByte(EB);
      } else if (integer == -1) {
       writeByte(EP);
        writeByte(EP); 
       writeByte(HB);
        writeByte(SB);
        writeByte(RecPacket + diff_rec); // Transmit one byte
        writeByte(EB);
      }
    }}
}

uint8_t best_SC_Rx(uint16_t v_R1, uint16_t v_R2, uint16_t v_R3)
{
v_R = max(v_R1, max(v_R2, v_R3));
return v_R;
}

uint8_t EGC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t tkn,uint8_t num)
{

  Avg =(voltage_R1+voltage_R2+voltage_R3)/3;
  if(tkn==HIGH){
      if(Avg>=Th_voltage){countB1++;if(num==7){return countB1;}}
      else{countB0++;}
  }
      else if (tkn==LOW){
        if(Avg>=Th_voltage)
            {input++;
            //Binary shift to store another bit
            input=input<<1;
            }
              if(num==7){input=input>>1;
            return input;}
      }
   return 0; // Default return value
}

uint8_t EGC_Engine_1bit(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3)
{

  Avg =(voltage_R1+voltage_R2+voltage_R3)/3;
      if(Avg>=Th_voltage){return 1;}
      else{return 0;}
}


uint8_t MLC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t tkn,uint8_t num)
{
  int cnt0=0,cnt1=0;
  if(voltage_R1>=Th_voltage){cnt1++;}else{cnt0++;}
  if(voltage_R2>=Th_voltage){cnt1++;}else{cnt0++;}
  if(voltage_R3>=Th_voltage){cnt1++;}else{cnt0++;}
  if(tkn==HIGH){
      if(cnt1>cnt0){countB1++;if(num==7){return countB1;countB1=0;}}
      else{countB0++;}
  }
      else if (tkn==LOW){
        if(cnt1>=cnt0)
            {input++;
            //Binary shift to store another bit
            input=input<<1;
            }
              if(num==7){input=input>>1;
            return input;}
      }
   return 0; // Default return value
}

uint8_t MLC_Engine_1bit(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3)
{
  int cnt0=0,cnt1=0;
  if(voltage_R1>=Th_voltage){cnt1++;}else{cnt0++;}
  if(voltage_R2>=Th_voltage){cnt1++;}else{cnt0++;}
  if(voltage_R3>=Th_voltage){cnt1++;}else{cnt0++;}
  
      if(cnt1>cnt0){return 1;}
      else{return 0;}
}

uint8_t SC_Engine(uint16_t voltage_R1, uint16_t voltage_R2, uint16_t voltage_R3,uint8_t num)
{

    if(voltage_R1>=Th_voltage){cntR1_1++;}else{cntR1_0++;}
      if(voltage_R2>=Th_voltage){cntR2_1++;}else{cntR2_0++;}
      if(voltage_R3>=Th_voltage){cntR3_1++;}else{cntR3_0++;}
      if(num==7)
              {if(cntR1_1>=cntR2_1 && cntR1_1>=cntR3_1){return cntR1_1;BestRx=0;cntR1_1=0;cntR2_1=0;cntR3_1=0;}
                 if(cntR2_1>=cntR1_1 && cntR2_1>=cntR3_1){return cntR2_1;BestRx=1;cntR1_1=0;cntR2_1=0;cntR3_1=0;}
                 if(cntR3_1>=cntR2_1 && cntR3_1>=cntR1_1){return cntR3_1;BestRx=2;cntR1_1=0;cntR2_1=0;cntR3_1=0;}
                }
      return cntR1_1;
}

uint8_t SC_EngineReception(uint16_t voltage,uint8_t num)
{
  if(voltage> Th_voltage)
  {input++;
    //Binary shift to store another bit
    input=input<<1;
    }
      if(num==7){input=input>>1;
    return input;}
      return 0; // Default return statement in case none of the conditions are met
}

uint8_t SC_Engine_1bit(uint16_t voltage)
{
  if(voltage> Th_voltage)
  {return 1;}
  else{return 0;}
     
}

//Sending 1byte number
void writeByte(char decimal)
{
//converting the decimal value to binary and sending 8 bit information from MSB to LSB
 int i,binary;
 for(i=0;i<BITLENGTH;i++)
 {
  //int a= decimal % 2;
 binary = (int)decimal/FIRSTBIT; //Getting the first binary bit value
 decimal= (decimal & ((int)FIRSTBIT -1));//Setting the first bit to zero
 decimal=decimal<<1; //Shift all bits by one to left
 if(binary==TRUE)
 //if (a==1)
 {
 digitalWrite(LED,HIGH);
// delayMicroseconds(5);
 //digitalWrite(LED,LOW);
  //analogWrite(A0,255);  
 //Serial.print("1");
 }
 else
 {
 digitalWrite(LED,LOW);
 //delayMicroseconds(5);
 //digitalWrite(LED,HIGH);
 // analogWrite(A0,0);  
 //Serial.print("0");
 }
 //delay(DELAY);
 delayMicroseconds(1000);
 }
//Serial.println();
//digitalWrite(LED,LOW);
}

int inRange(float low, float high, float x)        
{        
 if (low <= x && x <= high)
 return 1;
 else
 return 0;         
}

void PulseMapping (char Tempdata)
{
if(Tempdata>=0){digitalWrite(LED,HIGH);delayMicroseconds(1000);digitalWrite(LED,LOW);delayMicroseconds(1000);}
else {digitalWrite(LED,LOW);delayMicroseconds(1000);digitalWrite(LED,HIGH);delayMicroseconds(1000);}  
if(Tempdata==1||Tempdata==-1){digitalWrite(LED,HIGH);delayMicroseconds(1000);digitalWrite(LED,HIGH);}
else if(Tempdata==2||Tempdata==-2){digitalWrite(LED,LOW);delayMicroseconds(1000);digitalWrite(LED,LOW);}
else if(Tempdata==3||Tempdata==-3){digitalWrite(LED,HIGH);delayMicroseconds(1000);digitalWrite(LED,LOW);}
else if(Tempdata==4||Tempdata==-4){digitalWrite(LED,LOW);delayMicroseconds(1000);digitalWrite(LED,HIGH);}
Tempdata=0;
}
