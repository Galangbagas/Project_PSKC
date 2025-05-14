// --- Pin motor kanan ---
#define IN1_R 25
#define IN2_R 33
#define ENA   32

// --- Pin motor kiri ---
#define IN1_L 27
#define IN2_L 26
#define ENB   14

// --- Sensor ultrasonik ---
#define TRIG_KANAN 4
#define ECHO_KANAN 2
#define TRIG_DEPAN 18
#define ECHO_DEPAN 5

// --- Konstanta kontrol ---
#define DIST_DEPAN 20       // Batas aman depan (cm)
#define DIST_KANAN 20       // Target jarak ke tembok kanan (cm)
#define SPEED_BASE 150      // Kecepatan dasar motor (PWM)
#define SPEED_MAX 255

// --- Parameter PID ---
float Kp = 0.8;   // Koefisien proporsional
float Ki = 0.0;   // Koefisien integral
float Kd = 0.2;   // Koefisien derivatif

unsigned long lastTime = 0; // Menyimpan waktu sebelumnya
float dt = 0;               // Perubahan waktu
float lastError = 0;        // Error sebelumnya untuk derivative
float integral = 0;         // Integral error
float derivative = 0;       // Derivative error
float output = 0;           // Output PID

// --- Fungsi baca sensor ultrasonik ---
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// --- Fungsi kontrol motor ---
void motorKanan(bool maju, int speed) {
  digitalWrite(IN1_R, maju);
  digitalWrite(IN2_R, !maju);
  ledcWrite(0, speed);  // Mengontrol motor kanan dengan PWM
}

void motorKiri(bool maju, int speed) {
  digitalWrite(IN1_L, maju);
  digitalWrite(IN2_L, !maju);
  ledcWrite(1, speed);  // Mengontrol motor kiri dengan PWM
}

// --- Setup awal ---
void setup() {
  Serial.begin(115200);

  pinMode(IN1_R, OUTPUT); pinMode(IN2_R, OUTPUT);
  pinMode(IN1_L, OUTPUT); pinMode(IN2_L, OUTPUT);

  ledcSetup(0, 1000, 8); ledcAttachPin(ENA, 0);
  ledcSetup(1, 1000, 8); ledcAttachPin(ENB, 1);

  pinMode(TRIG_KANAN, OUTPUT); pinMode(ECHO_KANAN, INPUT);
  pinMode(TRIG_DEPAN, OUTPUT); pinMode(ECHO_DEPAN, INPUT);

  lastTime = millis();
  
  // Motor menyala dengan motor kiri sedikit lebih cepat
  motorKanan(true, SPEED_BASE);  // Kecepatan motor kanan normal
  motorKiri(true, SPEED_BASE + 50);  // Motor kiri sedikit lebih cepat
}

// --- Loop utama ---
void loop() {
  // Waktu sekarang
  unsigned long currentTime = millis();
  
  // Hitung perubahan waktu (dt) dalam detik (mendapatkan selisih waktu dalam milidetik)
  dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime; // Update waktu sebelumnya
  
  // Baca jarak dari sensor
  long jarakDepan = readUltrasonic(TRIG_DEPAN, ECHO_DEPAN);
  long jarakKanan = readUltrasonic(TRIG_KANAN, ECHO_KANAN);

  // Cetak jarak di serial monitor
  Serial.print("Depan: "); Serial.print(jarakDepan);
  Serial.print(" cm, Kanan: "); Serial.print(jarakKanan); Serial.println(" cm");

  // Deteksi tembok depan
  if (jarakDepan < DIST_DEPAN) {
    // Belok kiri → kanan maju, kiri mundur
    motorKanan(false, SPEED_BASE);
    motorKiri(true, SPEED_BASE + 20);
  } else {
    // PID control untuk mengikuti tembok kanan
    float error = DIST_KANAN - jarakKanan;
    integral += error * dt;
    derivative = (error - lastError) / dt;
    output = Kp * error + Ki * integral + Kd * derivative;
    lastError = error;

    int speedKanan = SPEED_BASE - output;  // Motor kanan mengikuti PID
    int speedKiri  = SPEED_BASE + output;  // Motor kiri mengikuti PID

    // Batasi kecepatan motor agar tidak < 0 atau > 255
    speedKanan = constrain(speedKanan, 70, 255);
    speedKiri  = constrain(speedKiri, 70, 255);

    motorKanan(true, speedKanan);  // Motor kanan mengikuti perhitungan PID
    motorKiri(true, speedKiri);    // Motor kiri mengikuti perhitungan PID
  }

  delay(100); // Delay untuk stabilkan pembacaan sensor
}