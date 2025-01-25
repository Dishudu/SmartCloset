#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Данные для подключения к Wi-Fi
const char* ssid = "__";
const char* password = "__";

// OpenWeather API
const char* weatherApiKey = "267d65004cad35a77e03b813c3841d27";
const char* city = "Moscow";

// Yandex GPT API
const char* yandexApiKey = "AQVNy62kblNxfHwTVjtH70xpRjaLRVapCa_mhM45";
const char* yandexApiUrl = "https://llm.api.cloud.yandex.net/foundationModels/v1/completion";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Подключение к Wi-Fi
  Serial.print("Подключение к Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi подключен!");
  
  // Получение погоды
  String weatherData = getWeatherData();
  if (!weatherData.isEmpty()) {
    // Формирование текста для Yandex GPT
    String recommendation = getClothingRecommendation(weatherData);
    Serial.println("Рекомендация по одежде: " + recommendation);
  }
}

void loop() {
  // Ничего не делаем в loop
}

String getWeatherData() {
  HTTPClient http;
  String weatherUrl = String("http://api.openweathermap.org/data/2.5/weather?q=") + city +
                      "&appid=" + weatherApiKey + "&units=metric";

  http.begin(weatherUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();
      return payload;
    } else {
      Serial.println("Ошибка получения данных погоды: " + String(httpCode));
    }
  } else {
    Serial.println("Ошибка подключения к OpenWeather API");
  }

  http.end();
  return "";
}

String getClothingRecommendation(String weatherData) {
  // Разбор JSON с данными погоды
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, weatherData);
  if (error) {
    Serial.println("Ошибка разбора JSON: " + String(error.c_str()));
    return "Ошибка данных погоды";
  }

  float temperature = doc["main"]["temp"];
  int humidity = doc["main"]["humidity"];
  String weatherDescription = doc["weather"][0]["description"];

  // Формирование текста запроса
  String weatherText = "Сегодня в городе " + String(city) +
                       " температура " + String(temperature) + "°C, влажность " +
                       String(humidity) + "%, погода: " + weatherDescription +
                       ". На основе этих данных предложите подходящую одежду из списка: куртка, пальто, свитер, футболка, шорты, джинсы, брюки, юбка, рубашка, водолазка, легкая куртка, перчатки, шарф, шапка, ботинки, кроссовки, сандалии, плащ.";

  // Отправка запроса в Yandex GPT
  HTTPClient http;
  http.begin(yandexApiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Api-Key " + String(yandexApiKey));

  // Формирование JSON-запроса
  DynamicJsonDocument requestBody(2048);
  requestBody["modelUri"] = "gpt://b1gs7501u463fdcul4i1/yandexgpt-lite";
  JsonObject completionOptions = requestBody.createNestedObject("completionOptions");
  completionOptions["stream"] = false;
  completionOptions["temperature"] = 1;
  completionOptions["maxTokens"] = 500;

  JsonArray messages = requestBody.createNestedArray("messages");
  JsonObject message = messages.createNestedObject();
  message["role"] = "user";
  message["text"] = weatherText;

  String requestBodyStr;
  serializeJson(requestBody, requestBodyStr);

  int httpCode = http.POST(requestBodyStr);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();

      // Разбор ответа Yandex GPT
      DynamicJsonDocument responseDoc(2048);
      error = deserializeJson(responseDoc, payload);
      if (error) {
        Serial.println("Ошибка разбора ответа Yandex GPT: " + String(error.c_str()));
        return "Ошибка данных от Yandex GPT";
      }

      String recommendation = responseDoc["result"]["alternatives"][0]["message"]["text"];
      return recommendation;
    } else {
      Serial.println("Ошибка получения данных от Yandex GPT: " + String(httpCode));
    }
  } else {
    Serial.println("Ошибка подключения к Yandex GPT API");
  }

  http.end();
  return "Нет ответа от Yandex GPT";
}
