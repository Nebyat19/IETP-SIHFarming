document.addEventListener("DOMContentLoaded", function () {
  const form = document.getElementById("cropForm");
  const resultContainer = document.getElementById("resultContainer");

  form.addEventListener("submit", async function (event) {
    event.preventDefault();

    const cropInput = document.getElementById("cropInput").value;
    const info = await fetchGenerativeInfo(cropInput);

    resultContainer.innerHTML = `<p>${info}</p>`;
  });

  async function fetchGenerativeInfo(crop) {
    const API_KEY = "AIzaSyAEuszrgOOh6-fM0UG4DkBW-ddZVxp2D0Y"; // Replace with your Google Generative AI API key
    const genAI = new GoogleGenerativeAI(API_KEY);
    const model = genAI.getGenerativeModel({ model: "gemini-pro" });

    try {
      const prompt = `Generate information about ${crop}.`;
      const result = await model.generateContent(prompt);
      const response = await result.response;
      const text = await response.text();
      return text;
    } catch (error) {
      console.error("Error fetching information:", error);
      return "An error occurred.";
    }
  }
});
