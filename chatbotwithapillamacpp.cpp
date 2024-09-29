#include "chatbotwithapillamacpp.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QFileInfo>

ChatbotWithAPILlamacpp::ChatbotWithAPILlamacpp(VizData *vizData, QObject *parent){
    this->vizData = vizData;

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ChatbotWithAPILlamacpp::handleAPIResponse);
}

void ChatbotWithAPILlamacpp::sendMessage(const QString& message) {

    QUrl url(vizData->chatbotData.apiProviderInfo[vizData->chatbotData.apiProvider][0]);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(vizData->chatbotData.apiProviderInfo[vizData->chatbotData.apiProvider][2]).toUtf8());

    // Append the new user message to the messages array
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    messagesArray.append(userMessage);

    // Prepare the JSON request body
    QJsonObject json;
    json["model"] = "gpt-4o-mini";  // Replace with the desired model
    json["messages"] = messagesArray;

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    // qDebug() << jsonDoc;

    // Send the request
    networkManager->post(request, jsonData);

    emit chatbotSendUserMessage(message);
}

void ChatbotWithAPILlamacpp::handleAPIResponse(QNetworkReply* reply) {
    // Handle the API response and append the assistant's response to the messages array
    QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject responseObject = responseDoc.object();

    // Extract assistant's message
    QJsonArray choicesArray = responseObject["choices"].toArray();
    if (!choicesArray.isEmpty()) {
        QString assistantResponse = choicesArray.first().toObject()["message"].toObject()["content"].toString();

        // qDebug() << assistantResponse;

        // Append assistant's response to the messages array
        QJsonObject assistantMessage;
        assistantMessage["role"] = "assistant";
        assistantMessage["content"] = assistantResponse;
        messagesArray.append(assistantMessage);

        emit chatbotResponseReceived(assistantResponse);
    } else {
        emit chatbotResponseReceived("Error: Invalid API response.");
    }

    reply->deleteLater();
}

void ChatbotWithAPILlamacpp::sendFile(const QString& filePath) {
    // Open and read the local file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Could not open file " << filePath;
        return;
    }

    QString fileContent = file.readAll();
    file.close();

    QUrl url(vizData->chatbotData.apiProviderInfo[vizData->chatbotData.apiProvider][0]);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(vizData->chatbotData.apiProviderInfo[vizData->chatbotData.apiProvider][2]).toUtf8());

    // Prepare the JSON request body with file content
    QJsonObject json;
    json["model"] = "gpt-4o-mini";  // Replace with the desired model
    QJsonArray messagesArray;

    // Add system message if needed
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a helpful assistant.";
    messagesArray.append(systemMessage);

    // Add the file content as user message
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = fileContent;  // Send file content as the message
    messagesArray.append(userMessage);

    // Add messages to the JSON object
    json["messages"] = messagesArray;

    // Convert JSON to QByteArray and send request
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkReply* reply = networkManager->post(request, jsonData);
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "API Response:" << response;
        } else {
            qDebug() << "Error in API request:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ChatbotWithAPILlamacpp::fromJS(const QString &message) {
    qDebug() << "Message from JavaScript:" << message;
}
