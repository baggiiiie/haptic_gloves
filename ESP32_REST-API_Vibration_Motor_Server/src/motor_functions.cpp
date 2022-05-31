#include <analogWrite.h>
const int rows = 4;
const int columns = 5;
int count1 = 0;
int count2 = 0;

/**
 * basic patterns:
 * touching:
 *    - human being/avatar: (single point: state > 100, freq. = constant ON)
 *    - table (all fingertips: state = random(80, 100), freq. = constant ON)
 *    - particle sun/cloud: (single point: state = random(80, 100), freq. = random ON/OFF)
 */

// ************************** Vibration Motor Patterns **************************
void logParams(int duration, int intensity) {
  Serial.print("Duration: ");
  Serial.println(duration);
  Serial.print("Intensity: ");
  Serial.println(intensity);
}
void turnAllMotorsOff(int motors[][columns]) {
  for (int i=0; i<rows; i++) {
    for (int j=0; j<columns; j++) {
      analogWrite(motors[i][j], 0); // OFF
    }
   }
}

void task1(void * parameters) {
  // infinite for-loop
  for(;;) {
    Serial.print("Task 1 count: ");
    Serial.println(count1++);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void task2(void * parameters) {
  // infinite for-loop
  for(;;) {
    Serial.print("Task 2 count: ");
    Serial.println(count2++);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  // int variance = 20; // %
  // int Nduration = duration * variance/2/100;
  // int Nintensity = intensity * variance/2/100;

  // int rndDuration = random(duration - Nduration, duration + Nduration);
  // int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
  // logParams(rndDuration, rndIntensity);
  // // infinite for-loop
  // for(;;) {
  //   for (int i=0; i<rows-1; i++) { // excluding back of hand
  //     for (int j=0; j<columns; j++) {
  //       analogWrite(motors[i][j], rndIntensity); // strong pulse ON, intensity: max. 250
  //     }
  //   }
  //   vTaskDelay(rndDuration / portTICK_PERIOD_MS); // normal duration: 120 ms
  //   turnAllMotorsOff(motors);
  //   vTaskDelay(rndDuration/1.2 / portTICK_PERIOD_MS);  // 100 ms
  //   for (int i=0; i<rows-1; i++) { // excluding back of hand
  //     for (int j=0; j<columns; j++) {
  //       analogWrite(motors[i][j], rndIntensity/2); // weak pulse ON
  //     }
  //   }
  //   vTaskDelay(rndDuration*1.9 / portTICK_PERIOD_MS);
  // }
}

void heartBeatTest(int motors[][columns], int intensity, int duration) {
  int variance = 20; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;

  int rndDuration = random(duration - Nduration, duration + Nduration);
  int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
  logParams(rndDuration, rndIntensity);

  for (int i=0; i<rows-1; i++) { // excluding back of hand
    for (int j=0; j<columns; j++) {
      analogWrite(motors[i][j], rndIntensity); // strong pulse ON, intensity: max. 250
    }
  }
  delay(rndDuration); // normal duration: 120 ms
  turnAllMotorsOff(motors);
  delay(rndDuration/1.2);  // 100 ms
  for (int i=0; i<rows-1; i++) { // excluding back of hand
    for (int j=0; j<columns; j++) {
      analogWrite(motors[i][j], rndIntensity/2); // weak pulse ON
    }
  }
  delay(rndDuration*1.9);  // 230 ms
  turnAllMotorsOff(motors);
  delay(rndDuration*4.6); // 450 ms
}


/**
 * Heartbeat: mode = 1 (non-sync version) / Clayton's version: sync w/ audio files
 * - intensity: 80-100
 * - duration: 120 ms
 * Ref. https://www.researchgate.net/figure/Most-common-waveform-of-the-classical-ECG-curve_fig1_325056456
 * http POST http://192.168.0.104/api/events time=99999 gpio=15 intensity=250 mode=1 duration=120 round=5
 */

// http POST 172.20.10.9/api/events time=99999 gpio=26 intensity=200 mode=1 duration=200 round=2

void heartBeatAll(int motors[][columns], int intensity, int duration, int round) {
  int variance = 20; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;
   for (int t=0; t<round; t++) {
     int rndDuration = random(duration - Nduration, duration + Nduration);
     int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
     logParams(rndDuration, rndIntensity);

      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], rndIntensity); // strong pulse ON, intensity: max. 250
        }
      }
      delay(rndDuration); // normal duration: 120 ms
      turnAllMotorsOff(motors);
      delay(rndDuration/1.2);  // 100 ms
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], rndIntensity/2); // weak pulse ON
        }
      }
      delay(rndDuration*1.9);  // 230 ms
      turnAllMotorsOff(motors);
      delay(rndDuration*4.6);  // 450 ms (*3.75) or 550 ms (*4.6)
    //   for (int i=0; i<rows-1; i++) { // excluding back of hand
    //     for (int j=0; j<columns; j++) {
    //       analogWrite(motors[i][j], rndIntensity/3.5); // late weak pulse ON
    //     }
    //   }
    //   delay(rndDuration);
    //   turnAllMotorsOff(motors);
    //   delay(rndDuration/1.2);
    }
}

/**
 * Raining: mode = 2
 * - intensity: 90
 *
 * TODO: make each sequence longer
*/
void iterateElements(int motors[][columns], int intensity, int duration, int round) {
  int variance = 30; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;
  for (int t=0; t<round; t++) {
    for (int i=0; i<(rows-1); i++) {
      for (int j=0; j<columns; j++) {
        int rndDuration = random(duration - Nduration, duration + Nduration);
        int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
        logParams(rndDuration, rndIntensity);
        analogWrite(motors[i][j], rndIntensity);
        delay(rndDuration);
        analogWrite(motors[i][j], 0);
        delay(rndDuration*0.8);
      }
    }
  }
  // turnAllMotorsOff(motors);
}


// Fingertips Spreed: mode = 3
void touchEachOther(int motors[][columns], int intensity, int duration, int round) {
  int variance = 20; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;
  for (int t=0; t<round; t++) {
    int rndDuration = random(duration - Nduration, duration + Nduration);
    int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
    logParams(rndDuration, rndIntensity);

    analogWrite(motors[0][2], rndIntensity); // middle finger: top knuckle
    delay(rndDuration);
    turnAllMotorsOff(motors);
    analogWrite(motors[0][1], rndIntensity/1.2); // pointer & ring finger: top knuckle
    analogWrite(motors[0][3], rndIntensity/1.2);
    delay(rndDuration*2);
    turnAllMotorsOff(motors);
    analogWrite(motors[0][0], rndIntensity/1.8); // thumb & littele finger: top knuckle
    analogWrite(motors[0][4], rndIntensity/1.8);
    delay(rndDuration*5);
    turnAllMotorsOff(motors);
  }
}
// Whole Hand Spreed: mode = 4
void spread(int motors[][columns], int intensity, int duration, int round) {
   int variance = 20; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;
  for (int t=0; t<round; t++) {
    int rndDuration = random(duration - Nduration, duration + Nduration);
    int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
    logParams(rndDuration, rndIntensity);

    analogWrite(motors[0][2], rndIntensity); // middle finger: top knuckle
    delay(rndDuration);
    turnAllMotorsOff(motors);
    analogWrite(motors[0][1], rndIntensity/1.2); // pointer & ring finger: top knuckle
    analogWrite(motors[0][3], rndIntensity/1.2);
    analogWrite(motors[1][2], rndIntensity/1.2);
    delay(rndDuration*2);
    turnAllMotorsOff(motors);
    analogWrite(motors[1][1], rndIntensity/1.8); // pointer & ring finger: middle knuckle
    analogWrite(motors[1][3], rndIntensity/1.8);
    analogWrite(motors[0][4], rndIntensity/1.8);
    analogWrite(motors[0][0], rndIntensity/1.8);
    delay(rndDuration*3);
    turnAllMotorsOff(motors);
    analogWrite(motors[1][0], rndIntensity/1.8);
    analogWrite(motors[1][4], rndIntensity/1.8);
    analogWrite(motors[2][2], rndIntensity/1.8);
    analogWrite(motors[2][0], rndIntensity/2);
    analogWrite(motors[2][3], rndIntensity/2);
    analogWrite(motors[2][1], rndIntensity/2.25);
    analogWrite(motors[2][4], rndIntensity/2.25);
    analogWrite(motors[3][0], rndIntensity/2.5);
    analogWrite(motors[3][1], rndIntensity/2.5);
    analogWrite(motors[3][2], rndIntensity/2.5);
    delay(rndDuration*5);
    turnAllMotorsOff(motors);
  }
}

// mode = 5
// TODO: replace single gpio to gpios[]
void iterateSingle(int pin, int intensity, int duration, int round) {
// void iterateSingle(int pins[], int intensity, int duration, int round) { // int pinsArr[] ??
  // int pinsSize = sizeof(pins)/sizeof(pins[0]);

  for (int t=0; t<round; t++) {
    // for (int i=0; i<pinsSize; i++) {
      // analogWrite(pins[i], random(70, intensity)); // amplitude

      // this for loop works correctly with an array of any type or size
      // for (byte i = 0; i < (sizeof(pins) / sizeof(pins[0])); i++) {
      //   analogWrite(pins[i], random(100, intensity));
      //   delay(random(20, duration)); // random
      // }

      analogWrite(pin, random(100, intensity));
      delay(random(20, duration)); // random

    // }
    // for (int i=0; i<pinsSize; i++) {
      // analogWrite(pins[i], 0); // OFF

      // this for loop works correctly with an array of any type or size
      // for (byte i = 0; i < (sizeof(pins) / sizeof(pins[0])); i++) {
      //   analogWrite(pins[i], 0); // OFF
      //   delay(random(20, duration)); // random
      // }

      analogWrite(pin, 0); // OFF
      delay(random(20, duration)); // random
    // }
  }
}

// mode = 6
// TODO: replace single gpio to gpios[]
void growSingle(int pin, int intensity, int duration, int round) {
  for (int t=0; t<round; t++) {
    for (int i=0; i<intensity; i++) {
      analogWrite(pin, i);
      delay(1);
      analogWrite(pin, 0); // OFF
      delay(1);
    }
    analogWrite(pin, 0); // OFF
    delay(duration); // random
  }
}
/**
 * Row Iteration: mode = 7
 * - intensity:
 *
 * TODO: make each sequence longer
*/
void iterateRow(int motors[][columns], int r, int intensity, int duration, int round) {
  for (int t=0; t<round; t++) {
    for (int j=0; j<columns; j++) {
      analogWrite(motors[r][j], intensity); // ON
        delay(duration);
      analogWrite(motors[r][j], 0); // OFF
        delay(duration);
    }
  }
}

/**
 * Magnetic water (rising): mode = 7
 * - intensity: 200
 * - duration: 2 ms
 * http POST http://192.168.0.104/api/events time=99999 gpio=15 intensity=200 mode=8 duration=2 round=2
 */
void goUpAll(int motors[][columns], int intensity, int duration, int round) {
  int variance = 20; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;

  for (int t=0; t<round; t++) {
    int rndDuration = random(duration - Nduration, duration + Nduration);
    int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
    // logParams(rndDuration, rndIntensity);

    for (int level=0; level<=intensity; level++) {
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], level); // strong pulse ON, intensity: max. 250
          logParams(rndDuration, level);
          // delay(rndDuration);
          delayMicroseconds(rndDuration);
        }
      }
    }
    // delay(rndDuration); // normal duration: 120 ms
    turnAllMotorsOff(motors);
  }
}

/**
 * Magnetic water (goUpDown): mode = 8
 * - intensity: 200
 * - duration: 2 microseconds
 * http POST http://192.168.0.104/api/events time=99999 gpio=15 intensity=200 mode=8 duration=2 round=2
 */
void goUpDownAll(int motors[][columns], int intensity, int duration, int round) {
  int variance = 80; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;

  for (int t=0; t<round; t++) {
    int rndDuration = random(duration - Nduration, duration + Nduration);
    int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
    // logParams(rndDuration, rndIntensity);

    // rising, TODO: random, lowest point: level=rand(60,?)
    for (int level=50; level<=intensity; level++) {
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], level); // strong pulse ON, intensity: max. 250
          logParams(rndDuration, level);
          // delay(rndDuration);
          delayMicroseconds(rndDuration);
        }
      }
    }
    // going down
    for (int level=intensity; level>=50; level--) {
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], level); // strong pulse ON, intensity: max. 250
          logParams(rndDuration, level);
          // delay(rndDuration);
          delayMicroseconds(rndDuration);
        }
      }
    }
  }
}

//  http POST 172.20.10.9/api/events time=99999 gpio=26 intensity=250 mode=10 duration=120 round=10
void rainingHeavily(int motors[][columns], int intensity, int duration, int round) {
  int variance = 30; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;
  for (int t=0; t<round; t++) {
    for (int i=0; i<(rows-1); i++) {
      for (int j=0; j<columns; j++) {
        int rndDuration = random(duration - Nduration, duration + Nduration);
        int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
        logParams(rndDuration, rndIntensity);
        analogWrite(motors[i][j], rndIntensity);
        delay(rndDuration);
        analogWrite(motors[i][j], 0);
        delay(rndDuration*0.8);
      }
    }
  }
  // turnAllMotorsOff(motors);
}
void snowMelting(int motors[][columns], int intensity, int duration, int round){
  int variance = 80; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;

  for (int t=0; t<round; t++) {
    int rndDuration = random(duration - Nduration, duration + Nduration);
    int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
    // logParams(rndDuration, rndIntensity);

    // rising, TODO: random, lowest point: level=rand(60,?)
    for (int level=50; level<=intensity; level++) {
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          analogWrite(motors[i][j], level); // strong pulse ON, intensity: max. 250
          logParams(rndDuration, level);
          // delay(rndDuration);
          delayMicroseconds(rndDuration);
        }
      }
    }
    for (int counter=0; counter<rows-1; counter++) {
      for (int i=0; i<rows-1; i++) { // excluding back of hand
        for (int j=0; j<columns; j++) {
          if (i==counter)
          {
            for (int level = intensity; level > 50; level--)
            {
              analogWrite(motors[i][j], level);
              logParams(rndDuration, level);
              delayMicroseconds(rndDuration*5);
            }
            
             // strong pulse ON, intensity: max. 250
            
            // delay(rndDuration);
            
          }
        }
      }
    }
    
  }
  turnAllMotorsOff(motors);
}
void raining(int motors[][columns], int intensity, int duration, int round) {
  int variance = 60; // %
  int Nduration = duration * variance/2/100;
  int Nintensity = intensity * variance/2/100;

  for (int t=0; t<round; t++) {
    for (int i=0; i<(rows-1); i++) {
      for (int j=0; j<columns; j++) {
        int rndDuration = random(duration - Nduration, duration + Nduration);
        int rndIntensity = random(intensity - Nintensity, intensity + Nintensity);
        logParams(rndDuration, rndIntensity);
        int motor_i [2] = {random(0, rows-1),random(0, rows-1)};
        int motor_j [2] = {random(0, columns),random(0, columns)};
        for (int counter = 0; counter < 2; counter++)
        {
          analogWrite(motors[motor_i[counter]][motor_j[counter]], 5*rndIntensity);
          // analogWrite(motors[motor_i[2-counter]][motor_j[2-counter]], 5*rndIntensity);
          delay(rndDuration*0.5);
        }
        for (int counter = 0; counter < 2; counter++)
        {
          analogWrite(motors[motor_i[counter]][motor_j[counter]], 0);
          // analogWrite(motors[motor_i[2-counter]][motor_j[2-counter]], 0);
        }
      }
    }
  }
  // turnAllMotorsOff(motors);
}