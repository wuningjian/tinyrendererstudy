#include "model.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

Model::Model(char* filename):verts(), faces(){
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) { // compare(pos, length, str) 匹配返回0
            //v -0.0871226 -0.186639 0.484069
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            //f 620/621/620 1257/1336/1257 619/619/619
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) { // isstringstream 重载的>>,会根据参数的类型确定读取的内容（这里是int和char）,idx是顶点index，另外两个itrash是vt和vn的index 目前只要了顶点坐标
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces.push_back(f);
        }
    }
    std::cerr << "# v# " << verts.size() << " f# "  << faces.size() << std::endl;
}

Model::~Model(){

}

int Model::nvert(){
    return (int)verts.size();
}

int Model::nface(){
    return (int)faces.size();
}

Vec3f Model::vert(int index){
    return verts[index];
}

vector<int> Model::face(int index){
    return faces[index];
}

void Model::PrintVert(){
    for(int i=0; i<verts.size(); i++){
        cout << verts[i] << endl;
    }
}

void Model::PrintFace(){

}