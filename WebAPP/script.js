// Firebase Configuration
const firebaseConfig = {
  apiKey: "AIzaSyDu47T1FhjtpTtz_tVeOUDKM9LXwoXHGus",
  authDomain: "voicewheelchair.firebaseapp.com",
  databaseURL: "https://voicewheelchair-default-rtdb.firebaseio.com",
  projectId: "voicewheelchair",
  storageBucket: "voicewheelchair.firebasestorage.app",
  messagingSenderId: "198672886571",
  appId: "1:198672886571:web:8e217d784adde54793736e",
  measurementId: "G-1C7QXDKDZT"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);
const commandsRef = firebase.database().ref("/wheelchair/commands");

// Function to send movement commands
function sendCommand(command) {
  const updates = {
    forward: false,
    back: false,
    left: false,
    right: false,
    stop: false
  };

  updates[command] = true;

  commandsRef.update(updates)
    .then(() => {
      document.getElementById("status").innerText = `Status: Command "${command}" sent successfully.`;
    })
    .catch((error) => {
      console.error("Error sending command:", error);
      document.getElementById("status").innerText = `Status: Failed to send command "${command}".`;
    });
}

// Function to update servo angles
function updateServo(servo, angle) {
  document.getElementById(`${servo}-angle`).innerText = `Angle: ${angle}°`;
  const updates = {};
  updates[servo] = parseInt(angle);
  
  if (servo === "servo1") {
    updates["servo11"] = parseInt(angle);
  }

  commandsRef.update(updates)
    .then(() => console.log(`Servo ${servo} updated to ${angle}°`))
    .catch((error) => console.error(`Error updating ${servo}:`, error));
}

// Function to update speed
function updateSpeed(speed) {
  document.getElementById("speed-value").innerText = `Speed: ${speed}`;
  commandsRef.update({ speed: parseInt(speed) })
    .then(() => console.log(`Speed updated to ${speed}`))
    .catch((error) => console.error("Error updating speed:", error));
}

// Accident Detection Listener
firebase.database().ref("/wheelchair/commands/accident").on("value", (snapshot) => {
  const accidentOccurred = snapshot.val();
  
  if (accidentOccurred) {
    hideAccidentWarning();
  } else {
    
    showAccidentWarning();
  }
});


// Function to show accident warning
function showAccidentWarning() {
  let statusElement = document.getElementById("status");
  statusElement.innerText = "⚠️ Warning: Accident Detected!";
  statusElement.style.color = "red";
  statusElement.style.fontWeight = "bold";
  statusElement.classList.add("warning");
}

// Function to hide accident warning
function hideAccidentWarning() {
  let statusElement = document.getElementById("status");
  statusElement.innerText = "All Clear: No accident detected.";
  statusElement.style.color = "green";
  statusElement.style.fontWeight = "bold";
  statusElement.classList.remove("warning");
}
