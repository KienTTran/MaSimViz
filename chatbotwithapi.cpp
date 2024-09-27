#include "chatbotwithapi.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QFileInfo>

ChatbotWithAPI::ChatbotWithAPI(const QString& apiKeyOrPath, QObject *parent)
    : ChatbotInterface(parent) {
    this->apiKey = getAPIKeyOrFile(apiKeyOrPath);
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ChatbotWithAPI::handleAPIResponse);
}

QString ChatbotWithAPI::getAPIKeyOrFile(const QString &apiKeyOrFile) {
    QFileInfo fileInfo(apiKeyOrFile);

    // Check if the input is a valid file path
    if (fileInfo.exists() && fileInfo.isFile()) {
        QFile file(apiKeyOrFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString apiKey = in.readAll().trimmed();  // Read the key from the file and trim whitespace
            file.close();
            return apiKey;
        } else {
            qWarning() << "Could not open the API key file.";
            return QString();
        }
    } else {
        // If it's not a file, return the input directly (assuming it's a key string)
        return apiKeyOrFile;
    }
}

void ChatbotWithAPI::sendMessage(const QString& message) {
    if (apiKey.isEmpty()) {
        emit chatbotResponseReceived("Error: API key not set.");
        return;
    }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject json;
    json["model"] = "gpt-4o-mini";  // Replace with the desired model
    QJsonArray messagesArray;
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    messagesArray.append(userMessage);
    json["messages"] = messagesArray;

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    networkManager->post(request, jsonData);

    emit chatbotSendUserMessage(message);
}

void ChatbotWithAPI::handleAPIResponse(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray choices = jsonObject["choices"].toArray();
        if (!choices.isEmpty()) {
            QString response = choices[0].toObject()["message"].toObject()["content"].toString();

            // qDebug() << response;

            emit chatbotResponseReceived(response);
        }
    } else {
        emit chatbotResponseReceived("Error: Failed to get a response from the API.");
    }
    reply->deleteLater();
}
