document.addEventListener("DOMContentLoaded", function () {
  const ledToggle = document.getElementById("ledToggle");
  const pumpToggle = document.getElementById("pumpToggle");
  const fanToggle = document.getElementById("fanToggle");
  const systemToggle = document.getElementById("systemToggle");

  ledToggle.addEventListener("change", function () {
    handleToggle(this, document.querySelector(".bi-lightbulb-fill"));
  });

  pumpToggle.addEventListener("change", function () {
    handleToggle(this, document.querySelector(".bi-circle-square"));
  });

  fanToggle.addEventListener("change", function () {
    handleToggle(this, document.querySelector(".bi-fan"));
  });

  systemToggle.addEventListener("change", function () {
    handleToggle(this, document.querySelector(".bi-lightbulb"));
  });

  function handleToggle(inputElement, iconElement) {
    if (inputElement.checked) {
      iconElement.style.color = "#28a745"; // Green when ON
    } else {
      iconElement.style.color = "#dc3545"; // Red when OFF
    }
  }
});
