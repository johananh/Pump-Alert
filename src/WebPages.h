// WebPages.h
#ifndef WEBPAGES_H
#define WEBPAGES_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pump Control</title>
    <style>
        body {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            font-family: Arial, sans-serif;
        }
        div {
            margin: 10px;
        }
        form {
            border: 1px solid #ccc;
            padding: 20px;
            display: inline-block;
        }
        #message {
            color: green;
            margin-top: 20px;
        }
    </style>
    <script>
        function validateOnTime(timeStr) {
            const parts = timeStr.split(':').map(x => parseInt(x, 10));
            return parts[0] < 60 && parts[1] < 60; // Validate minutes and seconds
        }

        function validateOffTime(timeStr) {
            const parts = timeStr.split(':').map(x => parseInt(x, 10));
            return parts[0] < 15 && parts[1] < 24; // Validate days and hours
        }

  function submitForm() {
    const onTimeout = document.getElementById('onTimeout').value;
    const offTimeout = document.getElementById('offTimeout').value;
    const dailyOkTime = document.getElementById('dailyOkTime').value;
    const formData = new FormData(document.forms[0]);
    fetch('/setParameters', {
        method: 'POST',
        body: formData
    }).then(response => {
        return response.text().then(text => {
            if (!response.ok) { // Check if the response status code is not OK (200)
                throw new Error(text);
            }
            return text; // Proceed as successful response
        });
    })
    .then(data => {
        document.getElementById('message').textContent = 'Parameters updated successfully!';
    })
    .catch(error => {
        // Use alert to display error messages
        alert("Error updating parameters: " + stripHtml(error.message));
        console.error('Error:', error);
    });
    return false; // Prevent form from submitting traditionally
}

// Function to strip HTML tags from error messages
function stripHtml(html) {
   let tmp = document.createElement("DIV");
   tmp.innerHTML = html;
   return tmp.textContent || tmp.innerText || "";
}

        function clearMessage() {
            document.getElementById('message').textContent = '';
        }

        document.addEventListener('DOMContentLoaded', function() {
            const inputs = document.querySelectorAll('input');
            inputs.forEach(input => {
                input.addEventListener('change', clearMessage);
            });
        });
    </script>
</head>
<body>
    <h1>Pump Settings</h1>
    <div>Max ON Time: {maxOnTime}</div>
    <div>Max OFF Time: {maxOffTime}</div>
    <form onsubmit="return submitForm();">
        <label for="onTimeout">Pump On Alert Timeout (MM:SS):</label>
        <input type="text" id="onTimeout" name="onTimeout" value="{onTimeout}" placeholder="MM:SS"><br><br>
        <label for="offTimeout">Pump Off Alert Timeout (DD:HH):</label>
        <input type="text" id="offTimeout" name="offTimeout" value="{offTimeout}" placeholder="DD:HH"><br><br>
        <label for="dailyOkTime">Daily OK Message Time (HH:MM):</label>
        <input type="text" id="dailyOkTime" name="dailyOkTime" value="{dailyOkTime}"placeholder="HH:MM"><br><br>
        <label for="trainMode">Training Mode:</label>
        <input type="checkbox" id="trainMode" name="trainMode" {trainModeChecked}><br><br>
        <input type="submit" value="Submit">
    </form>
    <div id="message"></div>
</body>
</html>
)rawliteral";

#endif // WEBPAGES_H
