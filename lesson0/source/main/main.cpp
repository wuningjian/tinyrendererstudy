#include "tgaimage.h"
#include "model.h"
#include <vector>
#include <iostream>
using namespace std;

const TGAColor red = TGAColor(0, 0, 255, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

//https://github.com/ssloy/tinyrenderer/wiki/Lesson-1-Bresenham%E2%80%99s-Line-Drawing-Algorithm
// first attempt 失真超级严重
// void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color){
//     for(float t=0.0; t<1.0;t+=0.01){
//         int x = x0 + (x1-x0)*t;
//         int y = y0 + (y1-y0)*t;
//         image.set(x, y, color);
//     }
// }

// // second attempt 用最小的x单位作为步长画线，效果比1好，但如果dx和dy差距大，线段中间会出现很大的空隙
// void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color){
//     for(int x=x0; x<=x1; x++){
//         float t = (x-x0)/(float)(x1-x0);
//         int y = y0*(1.0-t)+y1*t;
//         image.set(x, y, color);
//     }
// }

// Third attempt
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color){
    bool steep = false;
    if(abs(x0-x1)<abs(y0-y1)){
        swap(x0, y0);
        swap(x1, y1);
        steep = true;
    }
    if(x0>x1){
        swap(x0, x1);
        swap(y0, y1);
    }
    for(int x=x0; x<=x1; x++){
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.0-t)+y1*t;
        if(steep){
            image.set(y, x, color);
        }else{
            image.set(x, y, color);
        }
    }
}

// 后面的优化是对float，以及运算的优化（目前没看太懂）

const int width = 1024;
const int height = 1024;

int main(int argc, char** argv){
    Model *model = NULL;
    if(argc==2){
        model = new Model(argv[1]);
    }else{
        model = new Model("obj/african_head");
    }

    TGAImage image(width, height, TGAImage::RGBA);
    for(int i=0; i<model->nface(); i++){
        vector<int> faceData = model->face(i);
        for(int j=0; j<3; j++){
            Vec3f point1 = model->vert(faceData[j]);
            Vec3f point2 = model->vert(faceData[(j+1)%3]);
            int x0 = (point1.x+1.0)/2.*width;
            int y0 = (point1.y+1.0)/2.*height;
            int x1 = (point2.x+1.0)/2.*width;
            int y1 = (point2.y+1.0)/2.*height;
            //cerr << "(" << x0 << "," << y0 << "), " << "(" << x1 << "," << y1 << ")" << endl;
            line(x0, y0, x1, y1, image, green);
        }
    }
    image.flip_vertically();
    image.write_tga_file("out_african_head.tga");   
    // for(int i=30; i<60; i++){
    //     for(int j=30; j<60; j++){
    //         image.set(i, j, red);
    //     }
    // }
    // line(13, 20, 80, 40, image, green); 
    // line(20, 13, 40, 80, image, red); 
    // line(80, 40, 13, 20, image, red);
    // image.flip_vertically();
    // image.write_tga_file("out3.tga");
    return 0;
}

