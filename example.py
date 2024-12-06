import requests

# OpenWeather API
api_key = "267d65004cad35a77e03b813c3841d27"
city = "Moscow"
url = f"http://api.openweathermap.org/data/2.5/weather?q={city}&appid={api_key}&units=metric"

response = requests.get(url)
if response.status_code == 200:
    data = response.json()
    temperature = data['main']['temp']
    humidity = data['main']['humidity']
    weather_description = data['weather'][0]['description']

    # Формируем текст для Yandex GPT
    weather_text = (
        f"Сегодня в городе {city} температура {temperature}°C, "
        f"влажность {humidity}%, погода: {weather_description}. "
        f"На основе этих данных предложите подходящую одежду из списка: "
        f"куртка, пальто, свитер, футболка, шорты, джинсы, брюки, "
        f"юбка, рубашка, водолазка, легкая куртка, перчатки, шарф, "
        f"шапка, ботинки, кроссовки, сандалии, плащ."
        )
else:
    print("Ошибка получения данных погоды:", response.status_code)
    exit()

# Yandex GPT API
url = "https://llm.api.cloud.yandex.net/foundationModels/v1/completion"
headers = {
    "Content-Type": "application/json",
    "Authorization": "Api-Key AQVNy62kblNxfHwTVjtH70xpRjaLRVapCa_mhM45"
}
prompt = {
    "modelUri": "gpt://b1gs7501u463fdcul4i1/yandexgpt-lite",
    "completionOptions": {
        "stream": False,
        "temperature": 0.7,
        "maxTokens": "2000"
    },
    "messages": [
        {
            "role": "user",
            "text": weather_text
        }
    ]
}

response = requests.post(url, headers=headers, json=prompt)
if response.status_code == 200:
    result = response.json()
    alternatives = result.get("result", {}).get("alternatives", [])
    if alternatives:
        answer = alternatives[0].get("message", {}).get("text", "Нет ответа.")
    else:
        answer = "Модель не вернула результат."
    print(f"Рекомендация по одежде: {answer}")
else:
    print(f"Ошибка API Yandex GPT: {response.status_code}")
print(response.json())