#include <ServoTimer2.h>
#include <Wire.h>        // I2C (Wire) library - used for line tracking, color detection, ultrasonic on board A
#include "MH_TCS34725.h" // Color sensor library
#ifdef __AVR__
#include <avr/power.h>
#endif

// Color sensor initialization (integration time, gain)
MH_TCS34725 tcs = MH_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);
ServoTimer2 servo_pin_11; // servo: rotation
ServoTimer2 servo_pin_3;  // servo: arm
ServoTimer2 servo_pin_12; // servo: tray

void Forward1();
void Forward();
void Forward2();
void Forward3();
void Turn_left();
void Turn_right();
void Speedup();
void stopMotors();
void touzhi();
void yansexunji1();
void yansexunji();
void yanse();

int flag = 0; // general state flag
int m1, m2, m3;       // measured color values (reference)
int now1, now2, now3; // current color values
int a;
int b;

void setup() {
    pinMode(A0, INPUT);
    pinMode(A2, INPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);

    servo_pin_11.attach(11);
    servo_pin_3.attach(3);
    servo_pin_12.attach(12);

    // initial servo positions
    servo_pin_11.write(1540); // rotation servo - rear facing approx 82
    servo_pin_3.write(1400);  // arm servo initial
    servo_pin_12.write(1900); // tray servo
    delay(700);
    servo_pin_3.write(700);   // move arm
    delay(700);               // wait ~1s
    servo_pin_12.write(20);   // move tray
    delay(500);
    Serial.begin(9600);       // Serial output for debugging
}

void loop() {
    int a = analogRead(A0);
    int b = analogRead(A2);

    // debug prints (commented out)
    //Serial.println(a);
    //Serial.println(b);
    //delay(1000);

    // if in state 2 and both sensors read low (dark), use Forward2
    if (flag == 2 && a < 450 && b < 450) {
        Forward2();
    }

    // first check: encountered double black line (stop condition)
    if (flag == 0 && a < 400 && b < 400) {
        flag++;
        stopMotors();
        delay(800);
        Speedup();
        a = analogRead(A0);
        b = analogRead(A2);
    }

    a = analogRead(A0);
    b = analogRead(A2);
    // normal line-following logic
    if (a > 350 && b > 400) {
        Forward();
    }
    if (a > 350 && b < 400) {
        Turn_right();
    }
    if (a < 360 && b > 360) {
        Turn_left();
    }

    if (a > 350 && b > 400 && flag == 1) {
        Forward3();
    }
    if (flag == 1 && a < 280 && b < 280) {
        // stop motors briefly
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);

        delay(1000);

        // sweep to stabilize sensor readings (32 iterations)
        for (int e = 0; e < 32; e++) {
            //Serial.println(e);
            yansexunji();
            //Forward1();
            delay(20);
        }

        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
        delay(1000);

        // read the reference color (first measurement)
        uint16_t clear, red, green, blue; // store raw RGBC values
        tcs.getRGBC(&red, &green, &blue, &clear);
        tcs.lock(); // disable interrupt (optional)
        uint32_t sum = clear;
        float r, g, b;
        r = red;
        r /= sum;
        g = green;
        g /= sum;
        b = blue;
        b /= sum;
        r *= 256;
        g *= 256;
        b *= 256;
        delay(100);

        // read again to stabilize
        tcs.getRGBC(&red, &green, &blue, &clear);
        tcs.lock();
        sum = clear;
        r = red;
        r /= sum;
        g = green;
        g /= sum;
        b = blue;
        b /= sum;
        r *= 256;
        g *= 256;
        b *= 256;
        delay(100);
        m1 = r;
        m2 = g;
        m3 = b;

        // move to the first color card and sample
        int i = 0;
        while (i < 30) { // follow line and sample repeatedly
            yansexunji();
            delay(20);
            i++;
        }
        yanse();
        analogWrite(9, 0);
        analogWrite(10, 0);
        analogWrite(5, 0);
        analogWrite(6, 0);
        delay(1000);
        yanse();

        // compare measured reference (m*) with current (now*) to decide action
        if (m1 > m2 && m1 > m3 && now1 > now2 && now1 > now3) {
            // reference and first card are red
            touzhi();
            return;
        }
        if (m2 > m1 && m2 > m3 && now2 > now1 && now2 > now3) {
            // reference and first card are green
            touzhi();
            return;
        }
        if (m3 > m1 && m3 > m2 && now3 > now1 && now3 > now2) {
            // reference and first card are blue
            touzhi();
            return;
        }

        // proceed to second color card and sample
        int c = 0;
        while (c < 41) {
            yansexunji();
            delay(20);
            c++;
        }
        yanse();
        analogWrite(9, 0);
        analogWrite(10, 0);
        analogWrite(5, 0);
        analogWrite(6, 0);
        delay(1000);
        yanse();
        if (m1 > m2 && m1 > m3 && now1 > now2 && now1 > now3) {
            touzhi();
            return;
        }
        if (m2 > m1 && m2 > m3 && now2 > now1 && now2 > now3) {
            touzhi();
            return;
        }
        if (m3 >= m1 && m3 >= m2 && now3 >= now1 && now3 >= now2) {
            touzhi();
            return;
        }

        // proceed to third color card and sample
        int d = 0;
        while (d < 44) {
            yansexunji();
            delay(20);
            d++;
        }
        yanse();
        analogWrite(9, 0);
        analogWrite(10, 0);
        analogWrite(5, 0);
        analogWrite(6, 0);
        delay(1000);
        yanse();
        if (m1 > m2 && m1 > m3 && now1 > now2 && now1 > now3) {
            touzhi();
            return;
        }
        if (m2 > m1 && m2 > m3 && now2 > now1 && now2 > now3) {
            touzhi();
            return;
        }
        if (m3 > m1 && m3 > m2 && now3 > now1 && now3 > now2) {
            touzhi();
            return;
        }
    }
}

void Forward() {
    analogWrite(10, 120); // right motor forward
    analogWrite(9, 0);
    analogWrite(6, 255);  // left motor forward
    analogWrite(5, 0);
}

void Forward3() {
    analogWrite(10, 80);
    analogWrite(9, 0);
    analogWrite(6, 165);
    analogWrite(5, 0);
}


void Turn_left() {
    digitalWrite(10, HIGH);
    // analogWrite(10,255);
    analogWrite(9, 0);
    analogWrite(6, 0);
    analogWrite(5, 255); // turn left
}

void Turn_right() {
    analogWrite(10, 0);
    analogWrite(9, 180);
    digitalWrite(6, HIGH);
    // analogWrite(6,200);
    analogWrite(5, 0);
}


void Forward2() {
    analogWrite(10, 200);
    analogWrite(9, 0);
    analogWrite(6, 255);
    analogWrite(5, 0);
}

void stopMotors() {
    analogWrite(10, 0);
    analogWrite(9, 0);
    analogWrite(6, 0);
    analogWrite(5, 0);
}

void Speedup() {
    analogWrite(10, 140);
    analogWrite(9, 0);
    analogWrite(6, 255);
    analogWrite(5, 0);
    delay(2800);

    analogWrite(10, 0);
    analogWrite(9, 0);
    analogWrite(6, 0);
    analogWrite(5, 0);
    delay(500);

    analogWrite(10, 100);
    analogWrite(9, 0);
    analogWrite(6, 220);
    analogWrite(5, 0);
    delay(1400);
    // set tray servo
    servo_pin_12.write(1800);
}

void yanse() {
    uint16_t clear, red, green, blue; // store raw RGBC values
    tcs.getRGBC(&red, &green, &blue, &clear);
    tcs.lock(); // disable interrupt (optional)
    uint32_t sum = clear;
    float r, g, b;
    r = red;
    r /= sum;
    g = green;
    g /= sum;
    b = blue;
    b /= sum;
    r *= 256;
    g *= 256;
    b *= 256;
    now1 = r;
    now2 = g;
    now3 = b;
}

void Forward1() {
    analogWrite(10, 100);
    analogWrite(9, 0);
    analogWrite(6, 150);
    analogWrite(5, 0);
}

void yansexunji() {
    int a = analogRead(A0);
    int b = analogRead(A2);
    // sensor thresholds: a = left, b = right
    if (a > 350 && b > 350) { // both sensors see light surface
        Forward1(); // move forward
    }
    if (a > 350 && b < 350) { // right sensor sees dark -> move right
        Turn_right();
    }
    if (a < 350 && b > 350) { // left sensor sees dark -> move left
        Turn_left();
    }
}

void touzhi() {
    // perform projectile/ball release sequence
    flag = 2;
    servo_pin_12.write(700);
    delay(500);
    servo_pin_11.write(2200);
    delay(700);
    servo_pin_12.write(1900);
    delay(700);
    servo_pin_3.write(800);
    delay(1000);
    servo_pin_12.write(500);
    delay(500);
    servo_pin_11.write(1670); // final position for release
}
