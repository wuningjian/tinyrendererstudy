#ifndef __IMAGE_H__
#define __IMAGE_H__

#include<fstream>

#pragma pack(push,1)
//https://blog.csdn.net/m0_46338411/article/details/105747165 tga格式
struct TGA_Header{
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;
};
#pragma pack(pop)

struct TGAColor{
    union{
        struct{unsigned char b,g,r,a;};
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;
    TGAColor():val(0),bytespp(1){

    }
    TGAColor(unsigned char B, unsigned char G, unsigned char R, unsigned char A):
    b(B),g(G),r(R),a(A),bytespp(4){

    }
    TGAColor(int v, int bpp):val(v), bytespp(bpp){

    }
    TGAColor(const TGAColor &c):val(c.val), bytespp(c.bytespp){

    }
    TGAColor(const char *c, int bpp):val(0),bytespp(bpp){
        for(int i=0; i<bpp; i++){
            raw[i] = c[i];
        }
    }
    TGAColor & operator =(const TGAColor &c){
        if(this != &c){
            bytespp = c.bytespp;
            val = c.val;
        }
        return *this;
    }
};

class TGAImage{
protected:
    unsigned char*data;
    int width;
    int height;
    int bytespp;

    bool load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);
public:
    enum Format{GRAYSCALE=1, RGB=3, RGBA=4};//bytespp枚举值
    //各种参数构造方法
    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage &img);
    TGAImage & operator =(const TGAImage &image);
    ~TGAImage();
    bool read_tga_file(const char *filename);
    bool flip_horizontally();
    bool flip_vertically();
    bool set(int x, int y, TGAColor c);
    TGAColor get(int x, int y);
};

#endif //__IMAGE_H__