// Backend.h
#pragma once

#include <QObject>
#include <QImage>

class Backend : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QImage getRasterImage() const;
};
