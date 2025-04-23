#pragma once

#include <QQuickItem>
#include "VizData.h"

class MedianMapQuickItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(VizData* vizData READ vizData WRITE setVizData NOTIFY vizDataChanged)
public:
    MedianMapQuickItem();

    VizData* vizData() const { return vizData_; }
    void setVizData(VizData* data);

signals:
    void vizDataChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    VizData* vizData_ = nullptr;
    int currentMonth_ = 0;  // optional state
};
