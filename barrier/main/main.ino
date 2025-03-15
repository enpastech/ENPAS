#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "meto";      // SSID
const char* password = "metopass"; // Wi-Fi Password

WebServer server(80);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTOR_IN1 15
#define MOTOR_IN2 4
#define LED_PIN 27

int barrierState = 0;  // 0 = Upright, 1 = Down

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("ESP32's IP Address: ");
  Serial.println(WiFi.localIP());  // Print IP Address

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(LED_PIN, LOW);

  server.on("/", handleRoot);
  server.on("/ground", setBarrierGround);
  server.on("/stand", setBarrierStand);
  server.on("/stop", stopMotor);
  server.begin();
}

void loop() {
  server.handleClient();

  int distance = getDistance();
  //Serial.println("Distance: " + String(distance) + " cm");

  // Do not allow to move barrier upright if there is a vehicle on top of the barrier
  if (barrierState == 1 && distance < 20) {  
    Serial.println("Vehicle detected, barrier cannot move to upright!");
    delay(2000);
  }

  delay(500);
}

void handleRoot() {
  server.send(200, "text/html",
    "<h1>ENPAS Smart Barrier</h1>"
    "<a href='/ground'><button>Ground</button></a>"
    "<a href='/stand'><button>Upright</button></a>"
    "<a href='/stop'><button>Stop motor</button></a>");
}

void setBarrierStand() {
  if (barrierState != 0) {  // if the barrier is down
    int distance = getDistance();
    if (distance >= 20) {  // If there is no vehicle top of the barrier, upright the barrier
      Serial.println("Barrier is moving to upright...");
      blinkLED(3);
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, HIGH);
      delay(155);  // Time for rotating
      stopMotor();
      barrierState = 0;  // Barrier is upright
      server.send(200, "text/plain", "Barrier is upright");
    }
    else {
      Serial.println("Vehicle detected. barrier cannot move to upright!");
      server.send(200, "text/plain", "Vehicle detected. barrier cannot move to upright!");
    }
  }
  else {
    server.send(200, "text/plain", "Barrier is already upright");
  }
}

void setBarrierGround() {
  if (barrierState != 1) {  // If barrier is upright
    Serial.println("Barrier is moving to down...");
    blinkLED(3);
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    delay(155);  // Time for rotating
    stopMotor();
    barrierState = 1;  // Barrier is down
    server.send(200, "text/plain", "Barrier is down");
  }
  else {
    server.send(200, "text/plain", "Barrier is already down");
  }
}

void stopMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  Serial.println("Engine stopped");
  server.send(200, "text/plain", "Engine stopped");
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}
