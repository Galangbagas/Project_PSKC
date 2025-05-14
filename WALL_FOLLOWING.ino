#define IN1_R 27
#define IN2_R 26
#define ENA   14  // Motor kanan

#define IN1_L 25
#define IN2_L 33
#define ENB   32  // Motor kiri

#define TRIG_KANAN 4
#define ECHO_KANAN 2

#define TRIG_DEPAN 18
#define ECHO_DEPAN 5

#define DIST_DEPAN 20     // Jarak aman depan (cm)
#define DIST_KANAN 20     // Tembok kanan ideal
#define DEADZONE 5        // Zona toleransi kanan (±5 cm)
#define SPEED_BASE 150
#define SPEED_ADJUST 50

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void motorKanan(bool maju, int speed) {
  digitalWrite(IN1_R, maju);
  digitalWrite(IN2_R, !maju);
  ledcWrite(0, speed);
}

void motorKiri(bool maju, int speed) {
  digitalWrite(IN1_L, maju);
  digitalWrite(IN2_L, !maju);
  ledcWrite(1, speed);
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1_R, OUTPUT); pinMode(IN2_R, OUTPUT);
  pinMode(IN1_L, OUTPUT); pinMode(IN2_L, OUTPUT);
  ledcSetup(0, 1000, 8); ledcAttachPin(ENA, 0);
  ledcSetup(1, 1000, 8); ledcAttachPin(ENB, 1);
  pinMode(TRIG_KANAN, OUTPUT); pinMode(ECHO_KANAN, INPUT);
  pinMode(TRIG_DEPAN, OUTPUT); pinMode(ECHO_DEPAN, INPUT);
}

void loop() {
  long jarakDepan = readUltrasonic(TRIG_DEPAN, ECHO_DEPAN);
  long jarakKanan = readUltrasonic(TRIG_KANAN, ECHO_KANAN);

  Serial.print("Depan: "); Serial.print(jarakDepan);
  Serial.print(" cm, Kanan: "); Serial.print(jarakKanan); Serial.println(" cm");

  // Deteksi tembok depan
  if (jarakDepan < DIST_DEPAN) {
    // Belok kiri → kanan maju, kiri mundur
    motorKanan(true, SPEED_BASE);
    motorKiri(false, SPEED_BASE - SPEED_ADJUST);
  }
  // Jika tembok kanan terlalu dekat → koreksi kiri (perlambat motor kiri)
  else if (jarakKanan < DIST_KANAN - DEADZONE) {
    motorKanan(true, SPEED_BASE);
    motorKiri(true, SPEED_BASE - SPEED_ADJUST); // koreksi halus
  }
  // Jika tembok kanan terlalu jauh → koreksi kanan (perlambat motor kanan)
  else if (jarakKanan > DIST_KANAN + DEADZONE) {
    motorKanan(true, SPEED_BASE - SPEED_ADJUST); // koreksi kanan halus
    motorKiri(true, SPEED_BASE);
  }
  // Dalam zona aman tembok kanan (20 ± 5 cm)
  else {
    // Jalan lurus normal
    motorKanan(true, SPEED_BASE);
    motorKiri(true, SPEED_BASE);
  }

  delay(100);
}