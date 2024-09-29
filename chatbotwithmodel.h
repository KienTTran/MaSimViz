#ifndef CHATBOTWITHMODEL_H
#define CHATBOTWITHMODEL_H

#include "chatbotinterface.h"
#include "vizdata.h"

class ChatbotWithModel : public ChatbotInterface {
    Q_OBJECT

public:
    explicit ChatbotWithModel(VizData *vizData, QObject *parent = nullptr);

    // Implements the virtual method from ChatbotInterface
    void sendMessage(const QString& message) override;
    void sendFile(const QString& filePath) override;
    void fromJS(const QString& message) override;

private:
    VizData *vizData;
    QString modelPath;  // Path to the local model (e.g., for llama.cpp)

    // Simulated processing for the model (you would need to implement actual model interaction)
    QString processMessageWithModel(const QString& message);

signals:
    // Signal to notify when the chatbot has received a response
    void chatbotSendUserMessage(const QString& message);
    void chatbotResponseReceived(const QString& response);
};

#endif // CHATBOTWITHMODEL_H
