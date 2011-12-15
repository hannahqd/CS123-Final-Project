#ifndef HDR_IMAGE_H
#define HDR_IMAGE_H
#include <string>
#include <QVector2D>
//! Structure for holding data related to an .hdr image
struct HDR_Image
{
    int height;
    int width;
    float exposure;
    string format;
    QVector2D<BGRA> data;

};

#endif // HDR_IMAGE_H
