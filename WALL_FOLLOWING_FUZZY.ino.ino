#define ENA 14
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33
#define ENB 32

#define PWM_KIRI_CHANNEL 0
#define PWM_KANAN_CHANNEL 1

const int flamePin = 35;

const int PWM_BASE = 110;        // PWM dasar untuk maju
const int PWM_DELTA = 25;        // Koreksi dari fuzzy
const int PWM_SCAN = 130;        // PWM untuk scanning pelan
const int FLAME_STOP = 200;      // Jika sangat dekat api, berhenti
const int FLAME_DETECT = 4000;   // Di atas ini berarti api tidak terdeteksi

unsigned long lastScanTime = 0;
unsigned long lastWiggleTime = 0;
bool scanKanan = true;
bool wiggleRight = true;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🔥 Robot Fuzzy + Scan Kanan-Kiri Aktif");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttachPin(ENA, PWM_KIRI_CHANNEL);
  ledcSetup(PWM_KIRI_CHANNEL, 1000, 8);
  ledcAttachPin(ENB, PWM_KANAN_CHANNEL);
  ledcSetup(PWM_KANAN_CHANNEL, 1000, 8);
}

void loop() {
  static int flamePrev = analogRead(flamePin);
  delay(100);
  int flameNow = analogRead(flamePin);
  int error = flamePrev - flameNow;

  Serial.print("Flame: "); Serial.print(flameNow);
  Serial.print(" | Error: "); Serial.println(error);

  if (flameNow < FLAME_STOP) {
    Serial.println("🔥 Api sangat dekat! Robot berhenti.");
    berhenti();
    return;
  }

  if (flameNow < FLAME_DETECT) {
    // Api terdeteksi → maju sambil wiggle kanan-kiri kecil
    majuWiggleFuzzy(error, PWM_BASE);
  } else {
    // Api tidak terdeteksi → scan kanan kiri pelan
    scanApi();
  }

  flamePrev = flameNow;
}

// =========================
// Fuzzy Membership
// =========================

float membershipKiri(int x) {
  if (x <= -1000) return 1;
  if (x >= -100) return 0;
  return float(-x - 100) / 900.0;
}

float membershipTengah(int x) {
  if (x <= -200 || x >= 200) return 0;
  if (x >= -200 && x <= 0) return float(x + 200) / 200.0;
  if (x > 0 && x <= 200) return float(200 - x) / 200.0;
  return 0;
}

float membershipKanan(int x) {
  if (x >= 1000) return 1;
  if (x <= 100) return 0;
  return float(x - 100) / 900.0;
}

// =========================
// Gerak maju sambil wiggle kanan-kiri kecil
// =========================

void majuWiggleFuzzy(int error, int basePWM) {
  float muKiri = membershipKiri(error);
  float muTengah = membershipTengah(error);
  float muKanan = membershipKanan(error);

  float z = (
    muKiri * -PWM_DELTA +
    muTengah * 0 +
    muKanan * PWM_DELTA
  ) / (muKiri + muTengah + muKanan + 0.0001);

  // Timer wiggle
  unsigned long now = millis();
  if (now - lastWiggleTime > 500) {  // Ganti arah wiggle tiap 500 ms
    wiggleRight = !wiggleRight;
    lastWiggleTime = now;
  }

  int wiggleOffset = wiggleRight ? 15 : -15;  // PWM offset kecil untuk wiggle

  int pwmKiri = constrain(basePWM - z + wiggleOffset, 0, 255);
  int pwmKanan = constrain(basePWM + z - wiggleOffset, 0, 255);

  Serial.print("PWM Kiri: "); Serial.print(pwmKiri);
  Serial.print(" | PWM Kanan: "); Serial.println(pwmKanan);

  majuPWM(pwmKiri, pwmKanan);
}

// =========================
// Scan Movement (tidak ada perubahan)
// =========================

void scanApi() {
  unsigned long now = millis();
  if (now - lastScanTime > 1000) {
    scanKanan = !scanKanan;
    lastScanTime = now;
  }

  if (scanKanan) {
    Serial.println("🔍 Scanning ke kanan...");
    belokKanan(PWM_SCAN);
  } else {
    Serial.println("🔍 Scanning ke kiri...");
    belokKiri(PWM_SCAN);
  }
}

// =========================
// Motor Control
// =========================

void majuPWM(int kiri, int kanan) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(PWM_KIRI_CHANNEL, kiri);
  ledcWrite(PWM_KANAN_CHANNEL, kanan);
}

void belokKanan(int pwm) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  ledcWrite(PWM_KIRI_CHANNEL, pwm);
  ledcWrite(PWM_KANAN_CHANNEL, pwm);
}

void belokKiri(int pwm) {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(PWM_KIRI_CHANNEL, pwm);
  ledcWrite(PWM_KANAN_CHANNEL, pwm);
}

void berhenti() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(PWM_KIRI_CHANNEL, 0);
  ledcWrite(PWM_KANAN_CHANNEL, 0);
}