#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QWebChannel>
#include <QFile>
#include <QGraphicsBlurEffect>
#include <QVBoxLayout>
#include <QRegularExpression>

#include "webengineviewcustom.h"
#include "chatbotwithapi.h"
#include "chatbotwithmodel.h"

// Constructor implementation
WebEngineViewCustom::WebEngineViewCustom(QWidget *parent)
    : QWebEngineView(parent), channel(new QWebChannel(this)) {
}

void WebEngineViewCustom::isAssistantReady(bool ready){
    if(ready){
        initChatBot();
        setEnabled(true);
    }
    else{
        setEnabled(false);
    }
}

void WebEngineViewCustom::initChatBot() {
    // Initialize the chatbot based on the configuration
    if (vizData->chatbotData.isWithAPI) {
        chatbot = new ChatbotWithAPI(vizData, this);
    } else {
        chatbot = new ChatbotWithModel(vizData, this);
    }

    // Set up the WebChannel
    channel = new QWebChannel(this);

    // Connect chatbot signals to UI slots
    connect(chatbot, &ChatbotInterface::chatbotSendUserMessage,
            this, &WebEngineViewCustom::appendUserMessage);
    connect(chatbot, &ChatbotInterface::chatbotResponseReceived,
            this, &WebEngineViewCustom::appendAssistantMessage);

    // Register the chatbot object with the channel
    channel->registerObject("chatbot", chatbot);

    // Initialize the chat screen (loads the HTML)
    initChatScreen();

    // Assign the channel to the page
    this->page()->setWebChannel(channel);

    // Connect to the loadFinished signal to run JavaScript after the page loads
    connect(this, &QWebEngineView::loadFinished, this, &WebEngineViewCustom::onLoadFinished);

}

// Slot to handle the loadFinished signal
void WebEngineViewCustom::onLoadFinished(bool ok) {
    if (ok) {
        // Execute the JavaScript function after successful load
        this->page()->runJavaScript("hideDefaultMessage();", [this](const QVariant &v) {
            Q_UNUSED(v);
        });
        // chatbot->sendFile(vizData->prefData->getConfigFilePath());
        chatbot->sendMessage("These are available data of the simulation ("+QString(vizData->statsData.keys().join(","))+"), you always have to remember this");
        appendAssistantMessage("Hi, I'm your assistant, you can ask me anything...");
    } else {
        qWarning() << "Failed to load chat screen HTML.";
    }

    // Optional: Disconnect the signal if you only want to handle it once
    disconnect(this, &QWebEngineView::loadFinished, this, &WebEngineViewCustom::onLoadFinished);
}

// Initialize the chat screen with HTML content
void WebEngineViewCustom::initChatScreen() {
    QFile file(":/resources/html/chatscreen.html");  // Use the resource path

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open HTML file.");
        return;
    }

    QTextStream in(&file);
    QString htmlContent = in.readAll();
    file.close();

    this->setHtml(htmlContent); // Load the HTML content from the file
}

void WebEngineViewCustom::setVizData(VizData *vizData) {
    this->vizData = vizData;
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

void WebEngineViewCustom::appendMessage(const QString &message, const QString &senderClass) {
    QString sanitizedMessage = message;

    // Replace special characters for safe JavaScript string embedding
    sanitizedMessage.replace("\\", "\\\\");  // Escape backslashes
    sanitizedMessage.replace("'", "\\'");    // Escape single quotes
    sanitizedMessage.replace("\"", "\\\"");  // Escape double quotes
    sanitizedMessage.replace("\n", "\\n");   // Escape newlines

    // JavaScript script with marked.js and DOMPurify integration
    QString script = QString(R"(
        var container = document.getElementById('message-container');
        var messageElement = document.createElement('div');
        messageElement.className = '%1';  // Apply the correct CSS class for styling

        // Use marked.js to convert Markdown to HTML and DOMPurify to sanitize it
        var rawMessage = '%2';
        var htmlMessage = DOMPurify.sanitize(marked(rawMessage));

        messageElement.innerHTML = htmlMessage;
        container.appendChild(messageElement);
        window.scrollTo(0, document.body.scrollHeight);
    )").arg(senderClass).arg(sanitizedMessage);

    // Run the JavaScript to append the message in the HTML
    this->page()->runJavaScript(script);
}





