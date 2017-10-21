#include <Servo.h>

#ifdef ONLINE

#include <UIPEthernet.h>
#include <utility/logging.h>
EthernetClient ethClient;
#include <PubSubClient.h>

IPAddress MQTT_SERVER(192, 168, 3, 110);
const int MQTT_PORT = 1883;

const char* TOPIC_LED_STATUS = "led";
const char* TOPIC_LED_COMMAND = "led/toggle";

const char* TOPIC_GATE_STATUS = "gate";
const char* TOPIC_GATE_COMMAND = "gate/toggle";


PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, ethClient);
#endif

void callback(char* topic, byte* payload, unsigned int length) {
  char* payloadCharPtr = payload;
  payloadCharPtr[length] = 0;
  String payloadStr = String(payloadCharPtr);
  int payloadInt = payloadStr.toInt();
  
  String topicStr = String(topic);
  
  Serial.print("Topic received: "); Serial.println(topic);
  Serial.print("Message: "); Serial.println(msg);

  Serial.flush();

  if(topic == TOPIC_LED_COMMAND) {
    setLight(payloadInt);
  } else if (topic == TOPIC_GATE_COMMAND) {
    setGate(payloadInt);
  }
}

// Atualizar ultimo valor para ID do seu Kit para evitar duplicatas
const byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x42 };

const int LED_PIN = 2;
const int SERVO_PIN = 3;
Servo servo;

const int BTN_LED_PIN = 6;
const int BTN_GATE_PIN = 7;

int prevBtnLedState = LOW;
int prevBtnGateState = HIGH;

void setLight(int state) {
  if (state) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void setGate(int state) {
  if (estado) {
    Servo.write(90);
    delay(1000);
  } else {
    Servo.write(0);
    delay(1000);
  }
  updateStatus(DEVICE_GATE, state);
}

const int DEVICE_GATE = 0;
const int DEVICE_LED = 1;
void updateStatus(int device, int state) {
  char* topic;
  switch (device) {
    case DEVICE_GATE:
      topic = TOPIC_GATE_STATUS;
      break;
    case DEVICE_LED:
      topic = TOPIC_LED_STATUS;
      break;
  }

#ifdef ONLINE
  client.publish(topic, state);
#else
  Serial.print("Status update: "); Serial.print(topic);
  Serial.print(" | "); Serial.println(state);
#endif

}

#ifndef ONLINE
void SerialEvent() {
  // mensagem serial seria "{topico}|{payload}\n"
  while (Serial.available()) {
    String topic = Serial.readStringUntil('|');
    String payload = Serial.readStringUntil('\n');
    payload.trim();
    callback(topic.c_str(), payload.c_str(), payload.length());
  }
}
#endif

void setup()
{
  Serial.begin(9600);
  while (!Serial) {}

#ifdef ONLINE

  if (!Ethernet.begin(mac)) {
    Serial.println("DHCP Failed");
  } else {
    Serial.println(Ethernet.localIP());
  }
  Serial.println("Connecting...");
  char* clientId = "arduino-42";

  if (client.connect(clientId)) {
    Serial.println("Connected");
    // Se inscreve nos tópicos para que mensagens futuras possam ser
    // processadas através da função de callback
    client.subscribe(TOPIC_LED_COMMAND);
    client.subscribe(TOPIC_GATE_COMMAND);
  } else {
    Serial.println("Failed to connect to MQTT server");
  }
#endif

  Servo.attach(SERVO_PIN);
}

void loop()
{
  client.loop();
}
