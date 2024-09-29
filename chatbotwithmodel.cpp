#include "chatbotwithmodel.h"
#include <llama.h>

#include <QDebug>

ChatbotWithModel::ChatbotWithModel(VizData *vizData, QObject *parent)
    : ChatbotInterface(parent) {
    this->vizData = vizData;
    modelPath = vizData->chatbotData.modelPath;
}

void ChatbotWithModel::sendMessage(const QString& message) {
    // Process the message with the local model (replace this with actual model processing)
    QString response = processMessageWithModel(message);
    emit chatbotResponseReceived(response);
}


void ChatbotWithModel::fromJS(const QString& message) {
    // Process the message with the local model (replace this with actual model processing)
    QString response = processMessageWithModel(message);
    emit chatbotResponseReceived(response);
}


void ChatbotWithModel::sendFile(const QString& filePath) {
    // Process the message with the local model (replace this with actual model processing)
    QString response = processMessageWithModel(filePath);
    emit chatbotResponseReceived(response);
}

QString ChatbotWithModel::processMessageWithModel(const QString& message) {
    // Placeholder for model processing
    // In a real-world scenario, this would involve sending the message to a local LLM
    return "Simulated response from model for message: " + message;
}
