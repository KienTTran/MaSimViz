#ifndef CHATBOTWITHAPIOPENAI_H
#define CHATBOTWITHAPIOPENAI_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>

#include "vizdata.h"

// Class for OpenAI API-based chatbot implementation
class ChatbotWithAPIOpenAI : public QObject{
    Q_OBJECT

public:
    explicit ChatbotWithAPIOpenAI(VizData *vizData, QObject *parent = nullptr);

    // Implement sendMessage and sendFile
    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void sendFile(const QString& filePath);
    // From JavaScript interaction
    Q_INVOKABLE void fromJS(const QString& message);

    void handleAPIResponse(QNetworkReply* reply);

private:
    VizData *vizData;
    QNetworkAccessManager *networkManager;
    QJsonArray messagesArray;

signals:
    // Signal to notify when the chatbot has received a response
    void chatbotSendUserMessage(const QString& message);
    void chatbotResponseReceived(const QString& response);
};

#endif // CHATBOTWITHAPIOPENAI_H
