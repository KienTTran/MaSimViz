#include "chatbotwithapi.h"

ChatbotWithAPI::ChatbotWithAPI(VizData *vizData, QObject *parent)
    : ChatbotInterface(parent) {
    this->vizData = vizData;
    if(vizData->chatbotData.apiProvider == "OpenAI"){
        chatbotWithAPIOpenAI = new ChatbotWithAPIOpenAI(vizData, this);
        connect(chatbotWithAPIOpenAI, &ChatbotWithAPIOpenAI::chatbotSendUserMessage, this, &ChatbotWithAPI::chatbotSendUserMessage);
        connect(chatbotWithAPIOpenAI, &ChatbotWithAPIOpenAI::chatbotResponseReceived, this, &ChatbotWithAPI::chatbotResponseReceived);
    }
    if(vizData->chatbotData.apiProvider == "Llamacpp"){
        chatbotWithAPILlamacpp = new ChatbotWithAPILlamacpp(vizData, this);
        connect(chatbotWithAPILlamacpp, &ChatbotWithAPILlamacpp::chatbotSendUserMessage, this, &ChatbotWithAPI::chatbotSendUserMessage);
        connect(chatbotWithAPILlamacpp, &ChatbotWithAPILlamacpp::chatbotResponseReceived, this, &ChatbotWithAPI::chatbotResponseReceived);
    }
}

void ChatbotWithAPI::sendMessage(const QString& message) {
    if(vizData->chatbotData.apiProvider == "OpenAI") {
        chatbotWithAPIOpenAI->sendMessage(message);
    }
    if(vizData->chatbotData.apiProvider == "Llamacpp") {
        chatbotWithAPILlamacpp->sendMessage(message);
    }
}

void ChatbotWithAPI::sendFile(const QString& filePath) {
    if(vizData->chatbotData.apiProvider == "OpenAI") {
        chatbotWithAPIOpenAI->sendFile(filePath);
    }
    if(vizData->chatbotData.apiProvider == "Llamacpp") {
        chatbotWithAPILlamacpp->sendFile(filePath);
    }
}

void ChatbotWithAPI::fromJS(const QString &message) {
    if(vizData->chatbotData.apiProvider == "OpenAI") {
        chatbotWithAPIOpenAI->fromJS(message);
    }
    if(vizData->chatbotData.apiProvider == "Llamacpp") {
        chatbotWithAPILlamacpp->fromJS(message);
    }
}
