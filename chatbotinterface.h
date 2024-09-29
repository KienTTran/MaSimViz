#ifndef CHATBOTINTERFACE_H
#define CHATBOTINTERFACE_H

#include <QObject>

// Abstract class for chatbot interface
class ChatbotInterface : public QObject {
    Q_OBJECT

public:
    explicit ChatbotInterface(QObject *parent = nullptr) : QObject(parent) {}

    // Pure virtual function to be implemented by derived classes to send a message
    Q_INVOKABLE virtual void sendMessage(const QString& message) = 0;
    Q_INVOKABLE virtual void sendFile(const QString& filePath) = 0;
    Q_INVOKABLE virtual void fromJS(const QString& message) = 0;

signals:
    // Signal to notify when the chatbot has received a response
    void chatbotSendUserMessage(const QString& message);
    void chatbotResponseReceived(const QString& response);
};

#endif // CHATBOTINTERFACE_H
