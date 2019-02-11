 /*
  Firmware for the "2WD Ultrasonic Motor Robot Car Kit"

  Origin --------Stephen A. Edwards

  Hardware configuration :
  A pair of DC motors driven by an L298N H bridge motor driver
  An HC?'SR04 ultrasonic range sensor mounted atop a small hobby servo 
  PA added NewPing library- returns distance from background 
  PA added L298 library, compensation for stronger motor using speed ,,,,,,slower backing up no wheelie.
  
 // PA Jan21 2019 compiles
 ///////////////////////////Philduino Mods/////////////*/
 

 
 
#include <Servo.h>
#include <NewPing.h>      // Include New ping Library (downloaded zip)  
#include <L298N.h>        // AndreaLombardo Library from git (downloaded zip

// Servo motor that aims ultrasonic sensor .
#define servoPin 10 // PWM output for hobby servo
#define NUM_ANGLES 7
unsigned char sensorAngle[NUM_ANGLES] = { 30, 45, 60, 90, 110, 135, 165 };    // declare array  - datatype, array name. [number of elements]
unsigned int distance [NUM_ANGLES];											// declare array  - datatype, array name. [number of elements]
Servo servo;                                                                // instate a servo object named servo


// Ultrasonic Module 
#define trigPin A1								// 10 microsecond high pulse causes chirp , wait 50 us
#define echoPin  A2								// Width of high pulse indicates distance
#define Max_distance 250						 // ignore reflections further than 250mm ....about 10 inch.
NewPing sonar(trigPin, echoPin, Max_distance); // instantate a NewPing object sonar    setup of pins and maximum distance.


// Motor control pins : L298N H bridge
#define enAPin  3 // Left motor PWM speed control
#define in1Pin  4 // Left motor Direction 1
#define in2Pin  5 // Left motor Direction 2
#define in3Pin  8 // Right motor Direction 1
#define in4Pin  7 // Right motor Direction 2
#define enBPin  6 // Right motor PWM speed control


// Set motor speed: 255 full ahead, ?'255 full reverse , 0 stop void go( enum Motor m, int speed)
enum Motor { LEFT, RIGHT };
	
	

void go( byte m, int speed )
{
    digitalWrite(m == LEFT ? in1Pin : in3Pin , speed > 0 ? HIGH : LOW );
    digitalWrite(m == LEFT ? in2Pin : in4Pin , speed <= 0 ? HIGH : LOW );
    analogWrite(m == LEFT ? enAPin : enBPin, speed < 0 ? -speed : speed );
}

// Initial motor test :
// left motor forward then back // right motor forward then back
void testMotors(){
    static int 
        speed[8] = {128, 255, 128, 0, 128, 255, 128, 0};
    go(RIGHT, 0);
    
    for (unsigned char i = 0 ; i < 8 ; i++)
        go(LEFT, speed[i ]), delay (200);
    for (unsigned char i = 0 ; i < 8 ; i++)
        go(RIGHT, speed[i ]), delay (200); 
}
// Read distance from the ultrasonic sensor , return distance in cm //
 

unsigned int readDistance() {
 
 delay(30);                 // Wait 100ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
 int cm = sonar.ping_cm();   //Send ping, get ping distance in centimeters (cm).
 if (cm==0)
 {
	 cm=249;
 }
 //                                                                 int distance = cm;  Unused variable
 //Serial.print(" centemeters from readDistance");
 //Serial.println(cm);
 return cm;
}


// Scan the area ahead by sweeping the ultrasonic sensor left and right
// and recording the distance observed . This takes a reading , then
// sends the servo to the next angle . Call repeatedly once every 50 ms or so .
void readNextDistance() {
    static unsigned char angleIndex = 0;
    static signed char step = 1;
    distance [angleIndex] = readDistance ();
    angleIndex += step ;
    if (angleIndex == NUM_ANGLES - 1) 
        step = -1;
    else if (angleIndex == 0) 
        step = 1;
  
    servo.write( sensorAngle[angleIndex] );
	Serial.print ("sensor angle    "), Serial.print( sensorAngle[angleIndex] );
	Serial.print ("      distance     "), Serial.println( distance[angleIndex] );
	if (angleIndex == 3)
	Serial.println ("");
}

// Initial configuration
//
// Configure the input and output pins
// Center the servo
// Turn off the motors
// Test the motors
// Scan the surroundings once
//
void setup() {
	Serial.begin(115200);
  
    pinMode(enAPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(in3Pin, OUTPUT);
    pinMode(in4Pin, OUTPUT);
    pinMode(enBPin, OUTPUT);
    servo.attach( servoPin );
    servo.write(90);
	Serial.println("servo init 90 ");
	go(LEFT, 0);
	Serial.println("go left ");
    go(RIGHT, 0);
	Serial.println("go right ");
    testMotors ();
  	Serial.println("test motors ");
    // Scan the surroundings before starting
    servo.write( sensorAngle[0] );
    delay (200);
    for (unsigned char i = 0 ; i < NUM_ANGLES; i++)
        readNextDistance (), delay (200);
	Serial.println("scan angles setup");                     // debug
}

// Mainloop:
//
// Get the next sensor reading
// If anything appears to be too close , back up
// Otherwise, go forward
//
void loop() {
    readNextDistance ();
    // See if something is too close at any angle
    unsigned char tooClose = 0;
    for(unsigned char i = 0 ; i < NUM_ANGLES ; i++)
        if ( distance [i] < 40)
            tooClose = 1;

    if ( tooClose ) 
    {
        // Something's nearby: back up left
        go(LEFT, -180);
        go(RIGHT, -80);
    }
    else 
    {
        // Nothing in our way: go forward
        go(LEFT, 200);  // stronger motor 
        go(RIGHT, 255);
    }
    // Check the next direction in 50 ms
    delay (20);
}