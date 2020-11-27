#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tgaimage.h"

using namespace std;

TGAImage::TGAImage():data(NULL),width(0),height(0),bytespp(0){

}

TGAImage::TGAImage(int w, int h, int bpp):data(NULL), width(w), height(h), bytespp(bpp){
    unsigned long nbytes = width*height*bytespp;
    data = new unsigned char[nbytes];
    memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &image){
    width = image.width;
    height = image.height;
    bytespp = image.bytespp;
    unsigned long nbytes = width*height*bytespp;
    data = new unsigned char[nbytes];
    memcpy(data, image.data, nbytes);
}

TGAImage::~TGAImage(){
    if(data) delete[] data;
}

TGAImage & TGAImage::operator=(const TGAImage&image){
    if(this!=&image){
        width = image.width;
        height = image.height;
        bytespp = image.bytespp;
        unsigned long nbytes = width*height*bytespp;
        data = new unsigned char[nbytes];
        memcpy(data, image.data, nbytes);
    }
    return *this;
}

bool TGAImage::read_tga_file(const char *filename){
    //清空data
    if(data) delete [] data;
    data = NULL;
    //open file
    ifstream in;
    in.open(filename, ios::binary);
    if(!in.is_open()){
        cerr << "cannot open file " << filename << "\n";
        in.close();
        return false;
    }
    //read header
    TGA_Header header;
    in.read((char * )&header, sizeof(header));
    if(!in.good()){
        in.close();
        cerr << "cannot read tga header " << filename << "\n";
        return false;
    }
    //申请data内存空间
    width = header.width;
    height = header.height;
    bytespp = header.bitsperpixel>>3;//bits to bytes
    if(width<=0 || height<=0 || (bytespp!=GRAYSCALE || bytespp != RGB || bytespp!=RGBA)){
        cerr<<"error bytespp, width or height value!" << endl;
        in.close();
        return false;
    }
    unsigned long nbyte = width*height*bytespp;
    data = new unsigned char[nbyte];

    //编码 datatypecode 2 3
    if (3 == header.datatypecode || 2 == header.datatypecode){
        in.read((char*)data, nbyte);
        if(!in.good()){
            in.close();
            cerr << "an error occured while reading data\n";
            return false;
        }
    //编码 datatypecode 10 11
    }else if(10 == header.datatypecode || 11 == header.datatypecode){
        if(!load_rle_data(in)){
            in.close();
            cerr << "an error occured whild uncompress data\n";
            return false;
        }
    }else{
        in.close();
        cerr << "unknow file format " << (int)header.datatypecode << endl;
        return false;
    }
    //翻转 第五位代表从上到下
    if(!(header.imagedescriptor & 0x20)){

    }

    // 第四位代表从左到右
    if(header.imagedescriptor & 0x10){

    }

    cerr << width << "x" << height << "/" << bytespp*8 << endl;
    in.close();
    return true;
}

// rle压缩算法 https://blog.csdn.net/xiajun07061225/article/details/7646058
bool TGAImage::load_rle_data(ifstream &in){
    unsigned long pixelcount = width*height;
    unsigned long currentpixel = 0;
    unsigned long currentbyte = 0;
    TGAColor colorbuffer;
    do{
        unsigned char chunkheader = 0;
        chunkheader = in.get(); //取chunk第一bit，判断块数据类型，0：连续不重复颜色数据，1：连续重复颜色
        if(chunkheader < 128 ){
            // 第一个bit为0
            chunkheader++; // 因为数据范围0-127，代表有1-128个像素
            for(int i=0; i<chunkheader; i++){
                in.read((char * )colorbuffer.raw, bytespp);
                if(!in.good()){
                    cerr << "an error occured while reading the header\n";
                    return false;
                }
                for(int t=0; t<bytespp; t++){
                    data[currentbyte++] = colorbuffer.raw[t];
                }
                currentpixel++;
                if(currentpixel > pixelcount){
                    cerr << "pixel over range!" << endl;
                    return false;
                }
            }
        }else{
            // 第一个bit为1
            chunkheader -= 127; //-128+1
            in.read((char*)colorbuffer.raw, bytespp);
            if(!in.good()){
                cerr << "an error occured while reading the header\n";
                return false;
            }
            for(int i=0; i<chunkheader; i++){
                for(int t=0; t<bytespp;t++){
                    data[currentbyte++] = colorbuffer.raw[t];
                }
                currentpixel++;
                if(currentpixel > pixelcount){
                    cerr << "pixel over range!" << endl;
                    return false;
                }
            }
        }
    }while(currentpixel < pixelcount);
    return true;
}

bool TGAImage::set(int x, int y, TGAColor c){
    if(!data || x<0 || y<0 || x>=width || y>= height){
        cerr << "Invalid x,y are setted" << endl;
        return false;
    }
    memcpy(data+(x+y*width)*bytespp, c.raw, bytespp);
    return true;
}

TGAColor TGAImage::get(int x, int y) {
	if (!data || x<0 || y<0 || x>=width || y>=height) {
		return TGAColor();
	}
	return TGAColor((char *)data+(x+y*width)*bytespp, bytespp);
}

