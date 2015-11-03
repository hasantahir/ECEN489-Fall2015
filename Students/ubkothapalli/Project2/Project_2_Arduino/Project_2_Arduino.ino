#include <PID_v1.h>

#include <SoftwareSerial.h>


/*
 * Pin 04: Input from flow rate sensor
 * Pin 09: Get data on Teensy from NUC via BC-06. Directly connected to TXD of BC-06
 * Pin 10: Send data from Teensy to NUC via BC-06. Connected using voltage divider circuit.
 * Pin 14: IR input
 * Pin 15: Relay control output
 * Pin 20: PWM output for pump
 */

SoftwareSerial BTSerial(9,10);
//int Level1 = 16;
//int Level2 = 17;
int FlowSens = 4;
int RelayCtrl = 15;
int IRin = 14;
int PumpPWM = 20;
int powerLED = 23;


double Setpoint, Input;
double Output; 
float Kp = 2;
float Ki = 3;
float Kd = 1;

//const int sampleRate = 250;
PID myPID(&Input,&Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BTSerial.begin(9600);
  pinMode(FlowSens, INPUT);
  attachInterrupt(FlowSens, rpm, RISING);
  //pinMode(Level1, OUTPUT);
  //pinMode(Level2, OUTPUT);
  pinMode(PumpPWM, OUTPUT);
  pinMode(IRin, INPUT);
  pinMode(RelayCtrl, OUTPUT);
  myPID.SetMode(AUTOMATIC);
  //myPID.SetSampleTime(sampleRate);
  Setpoint = 1;
  
  myPID.SetOutputLimits(0, 250); 
  
  pinMode (powerLED,OUTPUT);

}

//initialize variables 
volatile int edges = 0;
int level = 0;
int count = 0;
float freq = 0;
float OutFlowRt = 0;
float InFlowRt = 0;
int SolState = 0;
bool RecData = false;
int commandSetpoint=6.5;
static String BTsend = "";
static String BTget = "";

//define calibration constants
const float Ap = -9.896452;
const float Bp =  53.94933;
const float Cp = -.7948750;
const float Dp =  15.38008;

//declare functions
int LpmToPWM(float LPerMin);
void GatherData();
void rpm ();
double voltToLevel(int levelSetpoint);


void loop() {
  
  digitalWrite(powerLED,HIGH);
    
    
    double IRvolt = analogRead(IRin);
    
    double IRtemp = map(IRvolt,0,650,0,400);    //Serial.print(IRvolt);
    //Serial.print("\t");
      delay (10);
    


    //Bluetooth communication
    if (BTSerial.available()){
      char in = BTSerial.read();
      if (!RecData){
        if (in == 's'){
          GatherData();
        } else if (in == 'g'){
          RecData = true;
        }
      } else {
        if (in == '\n'){
          commandSetpoint = BTget.toFloat();
          Setpoint = voltToLevel(commandSetpoint);
          BTget = "";
          RecData = false;
        } else {
          BTget += in;
        }
      }
    }

    

    Input = IRtemp;
   
    
    
     //Serial.print(level);
    //Serial.print("\t");
    myPID.Compute();
    Serial.print(Output);
    Serial.print("\t");
    int Output1 = map(Output,0,250,0,100);
    OutFlowRt = Output1;
  
    Serial.print(Input);
    Serial.print("\t");
    Serial.print((Setpoint));
    Serial.print("\t");
    
    //Serial.print(FlowSens);
    //Serial.print("\t");
    Serial.println(OutFlowRt);
    cli();
    freq = edges * 4;
    sei();
    count = 0;
    edges = 0;
  //}
  delay(500);
    analogWrite(PumpPWM, (OutFlowRt));

  //analogWrite(PumpPWM, LpmToPWM(OutFlowRt));
  digitalWrite(RelayCtrl, SolState);
}
//Define functions
void rpm(){
  edges = edges + 1;
}

//Convert from Liters per Minute to PWM value based on calibrated eqn.
int LpmToPWM(float LPerMin){
  float lpm;
  if (LPerMin > 3.5)
    lpm = 3.5;
  else if (LPerMin < 0)
    lpm = 0;
  else
    lpm = LPerMin;
  if(lpm ==0)
    return 0;
  else
    return (int) (Ap*pow(lpm,3) + Bp*pow(lpm,2) + Cp*lpm + Dp); 
}

void GatherData(){
  //BTsend should be in the order: IR range, Pump rate, Flow rate, Solenoid state
  //measure IR sensor
  //Serial.println(analogRead(IRin));
  if(analogRead(IRin) > 500)
    BTsend += '0';
  else
    BTsend += '1';
  BTsend += ',';
  
  //current output to pump
  BTsend += String(OutFlowRt);
  BTsend += ',';
  
  //Flow rate = frequency / 33 Hz/lpm
  InFlowRt = freq / 33;
  BTsend += String(InFlowRt);
  BTsend += ',';
  
  //current output to solenoid
  BTsend += String(SolState);
  BTSerial.println(BTsend);
}


double voltToLevel(int levelSetpoint){
  double dist;
  //int level1 = 0;
  dist = 1193.8* pow(levelSetpoint,-0.905);
  dist = map(dist,0,650,0,400);
  return dist;
}

