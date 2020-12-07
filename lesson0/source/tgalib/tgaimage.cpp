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
    if(width<=0 || height<=0 || (bytespp!= GRAYSCALE && bytespp != RGB && bytespp != RGBA)){
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
        flip_vertically();
    }

    // 第四位代表从左到右
    if(header.imagedescriptor & 0x10){
        flip_horizontally();
    }

    cerr << width << "x" << height << "/" << bytespp*8 << endl;
    in.close();
    return true;
}

bool TGAImage::write_tga_file(const char *filename, bool rle){
    unsigned char developer_area_ref[4] = {0, 0, 0, 0};
	unsigned char extension_area_ref[4] = {0, 0, 0, 0};
	unsigned char footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
    ofstream out;
    out.open(filename, ios::binary);
    if(!out.is_open()){
        cerr<<"can't open file "<<filename<<endl;
        out.close();
        return false;
    }
    TGA_Header header;
    memset((void*)&header, 0, sizeof(header));
    header.width = width;
    header.height = height;
    header.bitsperpixel = bytespp << 3;
    header.datatypecode = (bytespp==GRAYSCALE?(rle?11:3):(rle?10:2));
    header.imagedescriptor = 0x20;
    out.write((char*)&header, sizeof(header));
    if(!out.good()){
        out.close();
        cerr<<"can't dump the tga file "<< endl;
        return false;
    }
    if(!rle){
        out.write((char*)data, width*height*bytespp);
        if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
    }else {
		if (!unload_rle_data(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
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

bool TGAImage::unload_rle_data(std::ofstream &out) {
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width*height;
	unsigned long curpix = 0;
	while (curpix<npixels) {
		unsigned long chunkstart = curpix*bytespp;
		unsigned long curbyte = curpix*bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix+run_length<npixels && run_length<max_chunk_length) {
			bool succ_eq = true;
			for (int t=0; succ_eq && t<bytespp; t++) {
				succ_eq = (data[curbyte+t]==data[curbyte+t+bytespp]);
			}
			curbyte += bytespp;
			if (1==run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw?run_length-1:run_length+127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write((char *)(data+chunkstart), (raw?run_length*bytespp:bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
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

bool TGAImage::flip_horizontally(){
    if(!data) return false;
    int half = width >> 2;
    for(int i=0; i<half; i++){
        for(int j=0; j<height; j++){
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width-i-1, j);
            set(i, j, c2);
            set(width-i-1, j, c1);
        }
    }
    return true;
}

bool TGAImage::flip_vertically(){
    if(!data) return false;
    unsigned long byte_per_line = width * bytespp;
    int half = height >> 1;
    unsigned char * line = new unsigned char[byte_per_line];
    for(int i = 0; i<half; i++){
        unsigned long l1 = i*byte_per_line;
        unsigned long l2 = (width-1-i)*byte_per_line;
        memmove((void*)line, (void*)(data+l1), byte_per_line);
        memmove((void*)(data+l1), (void*)(data+l2), byte_per_line);
        memmove((void*)(data+l2), (void*)line, byte_per_line);
    }
    delete [] line;
    return true;
}

int TGAImage::get_width(){
    return width;
}

int TGAImage::get_height(){
    return height;
}