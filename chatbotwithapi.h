#ifndef CHATBOTWITHAPI_H
#define CHATBOTWITHAPI_H

#include "chatbotinterface.h"
#include "chatbotwithapiopenai.h"
#include "chatbotwithapillamacpp.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>

#include "vizdata.h"

class ChatbotWithAPI : public ChatbotInterface {
    Q_OBJECT

public:
    explicit ChatbotWithAPI(VizData *vizData, QObject *parent = nullptr);

    // Implements the virtual method from ChatbotInterface
    void sendMessage(const QString& message) override;
    void sendFile(const QString& filePath) override;
    void fromJS(const QString& message) override;

private:
    VizData *vizData;
    ChatbotWithAPIOpenAI *chatbotWithAPIOpenAI;
    ChatbotWithAPILlamacpp *chatbotWithAPILlamacpp;
};

#endif // CHATBOTWITHAPI_H
