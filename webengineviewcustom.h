#ifndef WEBENGINEVIEWCUSTOM_H
#define WEBENGINEVIEWCUSTOM_H

#include <QWebEngineView>
#include <QWebChannel>
#include <QNetworkAccessManager>
#include <QObject>
#include "vizdata.h"
#include "chatbotinterface.h"

class WebEngineViewCustom : public QWebEngineView {
    Q_OBJECT

public:
    // Constructor
    explicit WebEngineViewCustom(QWidget *parent = nullptr);

    // Function to append a user message styled as a blue chat bubble
    Q_INVOKABLE void appendUserMessage(const QString& message);

    // Function to append an assistant message styled as a gray chat bubble
    Q_INVOKABLE void appendAssistantMessage(const QString& message);

    void setVizData(VizData *vizData);

    void initChatBot();

private:
    // Helper function to inject JavaScript that appends a message to the chat
    void appendMessage(const QString& message, const QString& senderClass);

    // Initialize the chat window with HTML/CSS layout
    void initializeChatScreen();

    // Web channel object for C++ <-> JavaScript communication
    QWebChannel *channel;

    // Network manager for handling API requests (e.g., OpenAI API)
    QNetworkAccessManager *networkManager;

    VizData *vizData;

    ChatbotInterface *chatbot;
};

#endif // WEBENGINEVIEWCUSTOM_H
