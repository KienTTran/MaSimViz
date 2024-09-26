#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QWebChannel>

#include "webengineviewcustom.h"
#include "chatbotinterface.h"
#include "chatbotwithapi.h"
#include "chatbotwithmodel.h"

// Constructor implementation
WebEngineViewCustom::WebEngineViewCustom(QWidget *parent)
    : QWebEngineView(parent), channel(new QWebChannel(this)) {
}

void WebEngineViewCustom::initChatBot() {
    if(vizData->chatbotData.isWithAPI) {
        // Initialize the chatbot with API-based implementation
        chatbot = new ChatbotWithAPI(vizData->chatbotData.apiKey,this);
    } else {
        // Initialize the chatbot with model-based implementation
        chatbot = new ChatbotWithModel(vizData->chatbotData.modelPath, this);
    }

    initializeChatScreen();

    // Register the chatbot object to expose it to the WebChannel
    this->page()->setWebChannel(channel);
    channel->registerObject("chatbot", chatbot);

    // Connect the chatbot's response signal to display in the UI
    connect(chatbot, &ChatbotInterface::chatbotSendUserMessage, this, &WebEngineViewCustom::appendUserMessage);
    connect(chatbot, &ChatbotInterface::chatbotResponseReceived, this, &WebEngineViewCustom::appendAssistantMessage);
}

void WebEngineViewCustom::setVizData(VizData *vizData) {
    this->vizData = vizData;
}

// Initialize the chat with basic HTML and CSS for chat bubbles
void WebEngineViewCustom::initializeChatScreen() {
    QString initialHtml = R"(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #1c1c1c;
                color: #ffffff;
                margin: 0;
                padding: 0;
                height: 100vh;
                display: flex;
                flex-direction: column;
                overflow: hidden;
            }

            .message-container {
                flex: 1;
                padding: 10px;
                padding-top: 30px;
                display: flex;
                flex-direction: column;
                overflow-y: auto;
                background-color: #1c1c1c;
                color: #ffffff;
            }

            .user-message, .assistant-message {
                max-width: 70%;
                margin: 5px;
                padding: 8px;
                border-radius: 10px;
                word-wrap: break-word;
                font-size: 12px;
            }

            .user-message {
                align-self: flex-end;
                background-color: #007bff;
                color: white;
            }

            .assistant-message {
                align-self: flex-start;
                background-color: #444444;
                color: white;
            }

            .input-area {
                position: fixed;
                bottom: 0;
                left: 0;
                right: 0;
                display: flex;
                padding: 5px;
                background-color: #333333;
                border-top: 1px solid #555555;
                box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
            }

            .input-area input {
                flex: 1;
                padding: 5px;
                border-radius: 10px;
                border: 1px solid #666666;
                font-size: 12px;
                background-color: #444444;
                color: #ffffff;
            }

            .input-area button {
                margin-left: 5px;
                padding: 5px 10px;
                background-color: #007bff;
                color: white;
                border: none;
                border-radius: 10px;
                cursor: pointer;
                font-size: 12px;
            }

            .input-area button:hover {
                background-color: #0056b3;
            }

            /* Loading Spinner */
            .loading-spinner {
                border: 4px solid rgba(255, 255, 255, 0.3);
                border-top: 4px solid #007bff;
                border-radius: 50%;
                width: 20px;
                height: 20px;
                animation: spin 1s linear infinite;
            }

            @keyframes spin {
                0% { transform: rotate(0deg); }
                100% { transform: rotate(360deg); }
            }

            #loading-bar {
                display: flex;
                align-items: center;
                justify-content: center;
                flex: 1;
            }

            /* Theme Toggle Switch */
            .theme-toggle-container {
                position: fixed;
                top: 10px;
                left: 13px;
                display: flex;
                align-items: center;
                cursor: pointer;
                z-index: 1000;
            }

            .theme-toggle-label {
                margin-left: 5px;
                color: gray;
                font-size: 12px;
            }

            .theme-toggle {
                position: relative;
                width: 30px;
                height: 17px;
            }

            .theme-toggle input {
                opacity: 0;
                width: 0;
                height: 0;
            }

            .slider {
                position: absolute;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background-color: #ccc;
                transition: 0.4s;
                border-radius: 17px;
            }

            .slider:before {
                position: absolute;
                content: "";
                height: 13px;
                width: 13px;
                left: 2px;
                bottom: 2px;
                background-color: white;
                transition: 0.4s;
                border-radius: 50%;
            }

            input:checked + .slider {
                background-color: #007bff;
            }

            input:checked + .slider:before {
                transform: translateX(13px);
            }

            /* Light theme overrides */
            .light-theme body {
                background-color: #f5f5f5;
                color: #000000;
            }

            .light-theme .message-container {
                background-color: #f5f5f5;
                color: #000000;
            }

            .light-theme .assistant-message {
                background-color: #e1e1e1;
                color: black;
            }

            .light-theme .input-area {
                background-color: #ffffff;
                border-top: 1px solid #ddd;
            }

            .light-theme .input-area input {
                background-color: #ffffff;
                color: #000000;
            }

            .light-theme .theme-toggle-label {
                color: gray;
            }
        </style>
    </head>
    <body>
        <div class="theme-toggle-container">
            <label class="theme-toggle">
                <input type="checkbox" id="theme-toggle-checkbox">
                <span class="slider"></span>
            </label>
            <span class="theme-toggle-label" id="theme-label">Dark Mode</span>
        </div>

        <div class='message-container' id='message-container'></div>

        <div class='input-area'>
            <input type='text' id='message-input' placeholder='Type a message...'>
            <button id='send-button'>Send</button>
            <div id="loading-bar" style="display:none;">
                <div class="loading-spinner"></div>
            </div>
        </div>

        <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
        <script>
            new QWebChannel(qt.webChannelTransport, function(channel) {
                window.chatbot = channel.objects.chatbot;
            });

            function toggleLoading(isLoading) {
                var inputField = document.getElementById('message-input');
                var sendButton = document.getElementById('send-button');
                var loadingBar = document.getElementById('loading-bar');

                if (isLoading) {
                    inputField.style.display = 'none';
                    sendButton.style.display = 'none';
                    loadingBar.style.display = 'flex';
                } else {
                    inputField.style.display = 'block';
                    sendButton.style.display = 'block';
                    loadingBar.style.display = 'none';
                }
            }

            document.getElementById('send-button').addEventListener('click', function() {
                var inputField = document.getElementById('message-input');
                var message = inputField.value;
                if (message.trim() !== '') {
                    toggleLoading(true);  // Show loading spinner
                    window.chatbot.sendMessage(message);
                    inputField.value = '';
                }
            });

            document.getElementById('message-input').addEventListener('keypress', function(event) {
                if (event.key === 'Enter') {
                    document.getElementById('send-button').click();
                }
            });

            // Function to handle when the API response is received
            function onMessageReceived(message) {
                toggleLoading(false);  // Hide loading spinner and show input/button again
                console.log("Message from chatbot:", message);
            }

            // Theme toggle functionality
            document.getElementById('theme-toggle-checkbox').addEventListener('change', function() {
                var body = document.body;
                var themeLabel = document.getElementById('theme-label');
                if (this.checked) {
                    body.classList.add('light-theme');
                    themeLabel.innerText = 'Light Mode';
                } else {
                    body.classList.remove('light-theme');
                    themeLabel.innerText = 'Dark Mode';
                }
            });
        </script>
    </body>
    </html>

    )";

    this->setHtml(initialHtml); // Load the initial HTML content
}




void WebEngineViewCustom::appendUserMessage(const QString &message) {
    // Append a user message (right-aligned with a blue bubble)
    appendMessage(message, "user-message");
}

void WebEngineViewCustom::appendAssistantMessage(const QString &message) {
    // Append an assistant message (left-aligned with a gray bubble)
    appendMessage(message, "assistant-message");
    this->page()->runJavaScript("onMessageReceived();");
}

// This is the helper function that injects JavaScript to append messages
void WebEngineViewCustom::appendMessage(const QString &message, const QString &senderClass) {
    QString sanitizedMessage = message;

    // Replace newlines with <br> for HTML rendering
    sanitizedMessage.replace("\n", "<br>");

    // Escape any single quotes to prevent breaking JavaScript string syntax
    sanitizedMessage.replace("'", "\\'");

    QString script = QString(R"(
        var container = document.getElementById('message-container');
        var messageElement = document.createElement('div');
        messageElement.className = '%1';  // Apply the correct CSS class for styling
        messageElement.innerHTML = '%2';
        container.appendChild(messageElement);
        window.scrollTo(0, document.body.scrollHeight);
    )").arg(senderClass).arg(sanitizedMessage);

    // Run the JavaScript to append the message in the HTML
    this->page()->runJavaScript(script);
}



