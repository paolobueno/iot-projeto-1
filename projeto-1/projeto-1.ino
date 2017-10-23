#include <Servo.h>

#ifdef ONLINE
#include <PubSubClient.h>
#else
#include <SerialPubSubClient.h>
#endif

#include <UIPEthernet.h>
#include <utility/logging.h>
EthernetClient ethClient;

IPAddress MQTT_SERVER(192, 168, 3, 110);
const int MQTT_PORT = 1883;

const char* TOPIC_LED_STATUS = "led";
const char* TOPIC_LED_COMMAND = "led/toggle";

const char* TOPIC_GATE_STATUS = "gate";
const char* TOPIC_GATE_COMMAND = "gate/toggle";

const int DEVICE_GATE = 0;
const int DEVICE_LIGHT = 1;

void callback(char* topic, byte* payload, unsigned int length) {
  int payloadInt = payload[0] - '0';
  String topicStr = String(topic);

  if(topic == TOPIC_LED_COMMAND) {
    setLight(payloadInt);
  } else if (topic == TOPIC_GATE_COMMAND) {
    setGate(payloadInt);
  }
}

PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, ethClient);

// Atualizar ultimo valor para ID do seu Kit para evitar duplicatas
const byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x42 };

const int LED_PIN = 2;
const int SERVO_PIN = 3;
Servo servo;

const int BTN_LIGHT_PIN = 6;
const int BTN_GATE_PIN = 7;

int lightState = 0;
int gateState = 0;

void setLight(int state) {
  if (state) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  lightState = state;
  mqttUpdateStatus(DEVICE_LIGHT, state);
}

void setGate(int state) {
  if (state) {
    servo.write(90);
    delay(1000);
  } else {
    servo.write(0);
    delay(1000);
  }
  gateState = state;
  mqttUpdateStatus(DEVICE_GATE, state);
}

void toggleGate() {
  setGate(!gateState);
}

void toggleLight() {
  setLight(!lightState);
}

void mqttUpdateStatus(int device, int state) {
  const char* topic;
  switch (device) {
    case DEVICE_GATE:
      topic = TOPIC_GATE_STATUS;
      break;
    case DEVICE_LIGHT:
      topic = TOPIC_LED_STATUS;
      break;
  }
  client.publish(topic, state + '0');
}

void setup()
{
  Serial.begin(9600);
  while (!Serial) {}
  servo.attach(SERVO_PIN);

  #ifdef ONLINE
  if (!Ethernet.begin(mac)) {
    Serial.println("DHCP Failed");
  } else {
    Serial.println(Ethernet.localIP());
  }
  #endif

  Serial.println("Connecting...");
  const char* clientId = "arduino-42";

  if (client.connect(clientId)) {
    Serial.println("Connected");
    // Se inscreve nos tópicos para que mensagens futuras possam ser
    // processadas através da função de callback
    client.subscribe(TOPIC_LED_COMMAND);
    client.subscribe(TOPIC_GATE_COMMAND);
  } else {
    Serial.println("Failed to connect to MQTT server");
  }
}

int btnLightStatus = 0;
int btnGateStatus = 0;

void trackGateButton() {
  int status = digitalRead(BTN_GATE_PIN);
  if (status && !btnGateStatus) {
    toggleGate();
  }
  btnGateStatus = status;
}

void trackLightButton() {
  int status = digitalRead(BTN_LIGHT_PIN);
  if (status && !btnLightStatus) {
    toggleGate();
  }
  btnLightStatus = status;
}

void loop()
{
  client.loop();

  trackGateButton();
  trackLightButton();
}
