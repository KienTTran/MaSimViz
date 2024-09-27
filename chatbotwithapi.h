#ifndef CHATBOTWITHAPI_H
#define CHATBOTWITHAPI_H

#include "chatbotinterface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>

class ChatbotWithAPI : public ChatbotInterface {
    Q_OBJECT

public:
    explicit ChatbotWithAPI(const QString& apiKey, QObject *parent = nullptr);

    // Implements the virtual method from ChatbotInterface
    void sendMessage(const QString& message) override;

private slots:
    // Handle API response
    void handleAPIResponse(QNetworkReply* reply);

private:
    QString apiKey;  // API key for the chatbot service
    QNetworkAccessManager *networkManager;  // For sending HTTP requests
    QString getAPIKeyOrFile(const QString &apiKeyOrFile);

    QJsonArray messagesArray;  // Store all messages for the conversation
};

#endif // CHATBOTWITHAPI_H
