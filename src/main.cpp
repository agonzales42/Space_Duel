/*!
 * @author Alex Gonzales
 * @date 9/8/2022
 * "ingredients": LPD8806 LED strip, two buttons, two spaceships to place at the ends of the strip"
* @section About:
* 
* Space Duel is a turn-based, chance-based Arcade game where two players take turns firing salvos at eachother, decreasing the other's health.
* Red starts first game, loser starts subsequent games.
* Damage ranges from 0 to 100 hitpoints, so adjust "STARTING_HEALTH" to change the number of rounds for each game.
* Hitting 95 damage or better will trigger a Critical Hit! 1.5x modifier.
* The difficulty is controlled by "TRACKING_SPEED", which controls if and how well your shots will land.
* Good luck, and may the best captain win!
*/



#include <Arduino.h>
#include <LPD8806.h>

#define DATAPIN 14
#define CLOCKPIN 15
#define REDTRIGGER 13
#define GREENTRIGGER 12
#define REDTURN 3
#define GREENTURN 2
#define PIXELS 64    // Change depending on the length of the LED Strip
#define PULSE_LENGTH 8 // The actual length of the salvo in pixels
#define PULSE_DURATION 5000 // The time between pixel changes in microseconds. Lower for faster shots, higher for slower shots.

#define STARTING_HEALTH 300 // Increase for longer games
#define TRACKING_SPEED 75 // between 0 for always misses and 100 for always hits. 75 = 75% chance of hitting at all.

uint8_t turn = 'r';

LPD8806 strip = LPD8806(PIXELS, DATAPIN, CLOCKPIN);

void printAttack(String attacker, String defender, float_t damage, boolean didHit);
float_t calculateDamage();
uint8_t calculateExplosions(float_t damage);
void redFires(uint32_t c, uint8_t p, int t, float_t damage, boolean didHit);
void greenFires(uint32_t c, uint8_t p, int t, float_t damage, boolean didHit);
void gameOver(uint8_t w);


void setup() {
  pinMode(DATAPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(REDTURN, OUTPUT);
  pinMode(GREENTURN, OUTPUT);
  pinMode(REDTRIGGER, INPUT);
  pinMode(GREENTRIGGER, INPUT);
  strip.begin();
}

boolean newGame = true;
boolean didHit;
float_t greenHealth = STARTING_HEALTH, redHealth = STARTING_HEALTH;
float_t damage;

void loop() {
  if (turn == 'r') {
    digitalWrite(REDTURN, HIGH);
    digitalWrite(GREENTURN, LOW);
  } else {
    digitalWrite(REDTURN, LOW);
    digitalWrite(GREENTURN, HIGH);
  }
  if (newGame) {
    Serial.println("Let's play Space Duel!");
    if (turn == 'r') {
      Serial.println("Red goes first!");
    } else {
      Serial.println("Green goes first!");
    }
  }
  newGame = false;

  if (digitalRead(REDTRIGGER) == HIGH && turn == 'r') {

    damage = calculateDamage();
    didHit = damage > 0;

    redFires(0xff8800, PULSE_LENGTH, PULSE_DURATION, damage, didHit);

    if (didHit) {
      greenHealth = greenHealth - damage;
    }
    printAttack("Red", "Green", damage, didHit);
    if (greenHealth <= 0) {
      gameOver('r');
    }

    turn = 'g';

  } else if (digitalRead(GREENTRIGGER) == HIGH && turn == 'g') {

    damage = calculateDamage();
    didHit = damage > 0;

    greenFires(0xff8800, PULSE_LENGTH, PULSE_DURATION, damage, didHit);
    
    if (didHit) {
      redHealth = redHealth - damage;
    }
    printAttack("Green", "Red", damage, didHit);
    if (redHealth <= 0) {
      gameOver('g');
    }
    
    turn = 'r';
  }
}

void printAttack(String attacker, String defender, float_t damage, boolean didHit) {
  if (didHit) {
    Serial.print(attacker);
    Serial.print( " hit ");
    Serial.print(defender);
    Serial.print(" for ");
    Serial.print(damage, 2);
    Serial.print(" points! ");
    String hitQuality;
    switch (calculateExplosions(damage)) {
      case 9:
        hitQuality = "Beautiful shot, captain!";
        break;
      case 4: 
        hitQuality = "Excellent hit";
        break;
      case 3:
        hitQuality = "Great hit";
        break;
      case 2:
        hitQuality = "Good hit";
        break;
      case 1:
        hitQuality = "Weak hit";
        break;
    }
    Serial.print(hitQuality);
  } else {
    Serial.print(attacker);
    Serial.print(" missed ");
    Serial.print(defender);
  }
  Serial.print("!\nRed's health: ");
  Serial.print(redHealth, 2);
  Serial.print("  | Green's Health: ");
  Serial.print(greenHealth, 2);
  Serial.print("\n\n");
}

float_t calculateDamage() {
  uint8_t aim = TRACKING_SPEED;
  if (TRACKING_SPEED > 100) {
    aim = 100;
  } if (TRACKING_SPEED < 0) {
    aim = 0;
  }
  return ((float_t)100/aim)*random(100+1) - (((float_t)100-aim)*100/aim);
}

uint8_t calculateExplosions(float_t damage) {
  if (damage >= 95) {
    return (uint8_t)9;
  } else if (damage > 75) {
    return (uint8_t)4;
  } else if (damage > 50) {
    return (uint8_t)3;
  } else if (damage > 25) {
    return (uint8_t)2;
  }
  return (uint8_t)1;
}

void redFires(uint32_t c, uint8_t p, int t, float_t damage, boolean didHit) {
  if (c <= 0) {
    return;
  }
  for (int i = 0; i <= PIXELS; i++) {
    for(int j = i; j <= i+p; j++) {
      strip.setPixelColorRGB(j, (j*(c-1/(float_t)c) + 1/c));
    }
    strip.show();
    delayMicroseconds(t);
    strip.setPixelColorRGB(i, 0);
  }
  strip.show();
  if (didHit) {
    uint8_t explosions = calculateExplosions(damage);
    for(int k = 0; k < explosions; k++) {
      for (int j = PIXELS;  j >= PIXELS-6 && j >= PIXELS-explosions; j--) {
        strip.setPixelColorRGB(j, 0xff0000);
      }
      strip.show();
      if (explosions == 9) {
        delay(100);
      } else {
        delay(200);
      }

      for (int j = PIXELS;  j > PIXELS-6 && j >= PIXELS-explosions; j--) {
        strip.setPixelColorRGB(j, 0xffaa66);
      }
      strip.show();
      if (explosions == 9) {
        delay(100);
      } else {
        delay(200);
      }
    }
  
    for (int j = PIXELS;  j > PIXELS-6 && j >= PIXELS-explosions; j--) {
      strip.setPixelColorRGB(j, 0);
    }
    strip.show();
  }
}

void greenFires(uint32_t c, uint8_t p, int t, float_t damage, boolean didHit) {
  if (c <= 0) {
    return;
  }
  for (int i = PIXELS-p; i+p >= 0; i--) {
    for(int j = i; j <= i+p ; j++) {
      strip.setPixelColorRGB(j, (-1*j*(c-1/(float_t)c) + c));
    }
    strip.show();
    delayMicroseconds(t);
    strip.setPixelColorRGB(i+p, 0);
  }
  strip.show();
  if (didHit) {
    uint8_t explosions = calculateExplosions(damage);
    for(int k = 0; k < explosions; k++) {
      
      for(int j = 0; j < 5 && j < explosions; j++) {
        strip.setPixelColorRGB(j, 0xff0000);
      }
      strip.show();
      if (explosions == 9) {
        delay(100);
      } else {
        delay(200);
      }

      for(int j = 0;  j < 5 && j < explosions; j++) {
        strip.setPixelColorRGB(j, 0xffaa66);
      }
      strip.show();
      if (explosions == 9) {
        delay(100);
      } else {
        delay(200);
      }
    }
    for(int j = 0; j < 5 && j < explosions; j++) {
      strip.setPixelColorRGB(j, 0);
    }
    strip.show();
  }
}

void gameOver(uint8_t w) {
  uint32_t winnerColor;
  if (w == 'r') {
    Serial.print("\nGame over! Red wins!");
    winnerColor = 0xff0000;
  }
  if (w == 'g') {
    Serial.print("\nGame over! Green Wins!");
    winnerColor = 0x00ff00;
  }
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < PIXELS; j++) {
      strip.setPixelColorRGB(j, winnerColor);
    }
    strip.show();
    delay(250);
    for (int j = 0; j < PIXELS; j++) {
      strip.setPixelColorRGB(j, 0);
    }
    strip.show();
    delay(250);
  }
  Serial.println("\n\nNew Game!");
  redHealth = STARTING_HEALTH;
  greenHealth = STARTING_HEALTH;
  if ( w == 'r') {
    turn = 'g';
    Serial.println("Green goes first!");
  } else {
    turn = 'g';
    Serial.println("Red goes first!");
  }
}