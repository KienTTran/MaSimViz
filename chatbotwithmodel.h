#ifndef CHATBOTWITHMODEL_H
#define CHATBOTWITHMODEL_H

#include "chatbotinterface.h"

class ChatbotWithModel : public ChatbotInterface {
    Q_OBJECT

public:
    explicit ChatbotWithModel(const QString& modelPath, QObject *parent = nullptr);

    // Implements the virtual method from ChatbotInterface
    void sendMessage(const QString& message) override;

private:
    QString modelPath;  // Path to the local model (e.g., for llama.cpp)

    // Simulated processing for the model (you would need to implement actual model interaction)
    QString processMessageWithModel(const QString& message);
};

#endif // CHATBOTWITHMODEL_H
