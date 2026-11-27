const socket = new WebSocket(`ws://${window.location.host}/ws`);

const statusElement = document.getElementById("status");
const shakeElement = document.getElementById("shake-status");
const tiltElement = document.getElementById("tilt-direction");

socket.addEventListener("open", () => {
    statusElement.textContent = "Connected";
    statusElement.className = "connected";
});

socket.addEventListener("close", () => {
    statusElement.textContent = "Disconnected";
    statusElement.className = "";
});

socket.addEventListener("error", (error) => {
    statusElement.textContent = "Connection error";
    statusElement.className = "error";
    console.error("WebSocket error:", error);
});

socket.addEventListener("message", (event) => {
    try {
        const state = JSON.parse(event.data);
        updateDashboard(state);
    } catch (error) {
        console.error("Could not parse dashboard message:", error, event.data);
    }
});

function updateDashboard(state) {
    updateKeys(state.keys);
    updateTilt(state.direction);
    updateShake(state.shake);
}

function updateKeys(keys) {
    if (!Array.isArray(keys)) return;

    for (let index = 0; index < keys.length; index++) {
        const keyName = index.toString(16).toUpperCase();
        const keyElement = document.getElementById("key-" + keyName);

        if (keyElement) keyElement.classList.toggle("active", keys[index]);
    }
}

function updateTilt(direction) {
    if (!tiltElement || !direction) return;

    const activeDirections = [];

    if (direction.up) activeDirections.push("UP");
    if (direction.down) activeDirections.push("DOWN");
    if (direction.left) activeDirections.push("LEFT");
    if (direction.right) activeDirections.push("RIGHT");

    tiltElement.textContent = activeDirections.join(" + ") || "CENTER";
}

function updateShake(detected) {
    if (!shakeElement) return;

    shakeElement.textContent = detected ? "Shake detected" : "No shake detected";
    shakeElement.classList.toggle("is-detected", detected);
}