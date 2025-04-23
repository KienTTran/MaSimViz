// Main.qml - Fully GPU-accelerated raster visualization using Qt Quick and ShaderEffect
// import QtQuick 2.15
// import QtQuick.Window 2.15

// Window {
//     id: root
//     width: 800
//     height: 800
//     visible: true
//     title: "Raster Frequency Visualization"

//     property int rasterWidth: 100
//     property int rasterHeight: 100
//     property real minValue: 0.0
//     property real maxValue: 1.0

//     Image {
//         id: rasterSource
//         visible: false
//         source: "image://rasterdata"
//         sourceSize.width: 96
//         sourceSize.height: 131
//     }

//     ShaderEffect {
//         anchors.fill: parent
//         property variant rasterTexture: rasterSource
//         property real minValue: 0.0
//         property real maxValue: 0.2
//         vertexShader: "qrc:/qml/shaders/raster.vert"
//         fragmentShader: "qrc:/qml/shaders/raster.frag"
//     }

// }

// main.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import CustomComponents 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Raster Viewer")

    RasterImage {
        id: rasterView
        anchors.fill: parent
    }

    Component.onCompleted: {
        // Load the image from C++ backend
        var image = backend.getRasterImage(); // Assume this returns a QImage
        rasterView.image = image;
    }
}

