// Backend.cpp
#include "Backend.h"
#include "VizData.h" // Assuming this is your data source

QImage Backend::getRasterImage() const {
    // Assuming you have a method to create the raster image
    VizData vizData;
    return vizData.createRasterImage();
}
