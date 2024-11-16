#include <WiFi.h>
#include <PubSubClient.h>

// WiFi và MQTT thông tin
const char* ssid = "Pi";       // Tên WiFi
const char* password = "123456"; // Mật khẩu WiFi
const char* mqtt_server = "4dd631e7c9b946149f240234824a4e6b.s1.eu.hivemq.cloud"; // MQTT Broker
const char* mqtt_topic = "voltage log"; // Chủ đề gửi dữ liệu

WiFiClient espClient;
PubSubClient client(espClient);

// UART
#define UART_RX_PIN 16 // Chân RX của ESP32 (kết nối với TX của STM32)
#define UART_TX_PIN 17 // Chân TX của ESP32 (kết nối với RX của STM32)

// Serial interface cho UART
HardwareSerial UARTSerial(1); // UART1

// Queue FreeRTOS
QueueHandle_t dataQueue;

// Kích thước buffer cho dữ liệu
#define BUFFER_SIZE 50

// Hàm kết nối WiFi
void connectWiFi() {
  Serial.print("Kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối.");
}

// Hàm kết nối MQTT
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Kết nối MQTT...");
    if (client.connect("ESP32Client")) { // ID MQTT
      Serial.println("Đã kết nối MQTT.");
      client.subscribe(mqtt_topic); // Đăng ký chủ đề nếu cần nhận lệnh
    } else {
      Serial.print("Thất bại, mã lỗi: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// Task nhận dữ liệu từ UART
void uartTask(void* parameter) {
  char buffer[BUFFER_SIZE];
  while (true) {
    if (UARTSerial.available()) {
      String voltageData = UARTSerial.readStringUntil('\n'); // Đọc dữ liệu kết thúc bởi '\n'
      voltageData.trim(); // Loại bỏ ký tự trắng
      if (voltageData.length() > 0) {
        // Đưa dữ liệu vào queue
        snprintf(buffer, BUFFER_SIZE, "%s", voltageData.c_str());
        if (xQueueSend(dataQueue, buffer, portMAX_DELAY) == pdPASS) {
          Serial.println("Dữ liệu đưa vào queue: " + String(buffer));
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Kiểm tra UART mỗi 10ms
  }
}

// Task gửi dữ liệu lên MQTT
void mqttTask(void* parameter) {
  char buffer[BUFFER_SIZE];
  while (true) {
    // Nhận dữ liệu từ queue
    if (xQueueReceive(dataQueue, buffer, portMAX_DELAY) == pdPASS) {
      // Định dạng thành JSON
      char jsonPayload[BUFFER_SIZE];
      snprintf(jsonPayload, BUFFER_SIZE, "{\"voltage\": %s}", buffer);

      // Gửi qua MQTT
      if (client.connected()) {
        client.publish(mqtt_topic, jsonPayload);
        Serial.println("Dữ liệu đã gửi lên MQTT: " + String(jsonPayload));
      } else {
        Serial.println("MQTT chưa kết nối, bỏ qua.");
      }
    }
  }
}

void setup() {
  Serial.begin(115200); // Debug
  UARTSerial.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN); // UART giao tiếp với STM32
  
  connectWiFi();
  client.setServer(mqtt_server, 1883);
  
  // Tạo queue FreeRTOS
  dataQueue = xQueueCreate(10, BUFFER_SIZE); // Queue chứa tối đa 10 mục, mỗi mục 50 byte
  if (dataQueue == NULL) {
    Serial.println("Không tạo được queue!");
    while (1);
  }
  
  // Tạo các task
  xTaskCreate(uartTask, "UART Task", 2048, NULL, 1, NULL);
  xTaskCreate(mqttTask, "MQTT Task", 2048, NULL, 1, NULL);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop(); // Duy trì kết nối MQTT
}
