document.addEventListener('DOMContentLoaded', () => {
  const apiKey = "a8d503e79f791912910dc535060e0137"; // Replace with your OpenWeatherMap API key
  const query = 'Addis Ababa'; // Replace with the city name or query

  fetchWeatherData(apiKey, query);
});

async function fetchWeatherData(apiKey, query) {
  const apiUrl = `https://api.openweathermap.org/data/2.5/weather?q=${query}&appid=${apiKey}&units=metric`;

  try {
    const response = await fetch(apiUrl);
    const data = await response.json();

    if (response.ok) {
      displayWeatherInfo(data);
    } else {
      console.error('Error fetching weather data:', data.message);
    }
  } catch (error) {
    console.error('Error fetching weather data:', error);
  }
}

function displayWeatherInfo(weatherData) {
  const leftColumn = document.getElementById('leftColumn');
  const rightColumn = document.getElementById('rightColumn');

  leftColumn.innerHTML = `
    <p class="info"><i class="icon building"></i> City: <span class="value">${weatherData.name}</span></p>
    <p class="info"><i class="icon thermometer"></i> Temperature: <span class="value">${weatherData.main.temp}°C</span></p>
    <p class="info"><i class="icon cloud"></i> Weather Condition: <span class="value">${weatherData.weather[0].description}</span></p>
    <p class="info"><i class="icon tint"></i> Humidity: <span class="value">${weatherData.main.humidity}%</span></p>
    <p class="info"><i class="icon forward"></i> Wind Speed: <span class="value">${weatherData.wind.speed} m/s</span></p>
    <p class="info"><i class="icon rocket"></i> Pressure: <span class="value">${weatherData.main.pressure} hPa</span></p>
  `;

  rightColumn.innerHTML = `
    <p class="info"><i class="icon eye"></i> Visibility: <span class="value">${weatherData.visibility} meters</span></p>
    <p class="info"><i class="icon sun"></i> Sunrise: <span class="value">${new Date(weatherData.sys.sunrise * 1000).toLocaleTimeString()}</span></p>
    <p class="info"><i class="icon sun"></i> Sunset: <span class="value">${new Date(weatherData.sys.sunset * 1000).toLocaleTimeString()}</span></p>
    <p class="info"><i class="icon cloud"></i> Cloudiness: <span class="value">${weatherData.clouds.all}%</span></p>
  `;
}
