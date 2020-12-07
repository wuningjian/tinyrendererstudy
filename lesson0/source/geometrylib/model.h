#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
using namespace std;

struct faceData{
    vector<int> uvIndices;
    vector<int> normalIndices;
    vector<int> vertIndice;
};

class Model{
    private:
        vector<Vec3f> verts;
        vector<faceData> faces;
        vector<Vec3f> uvs;
        vector<Vec3f> normals;
    public:
        Model(char * filename);
        ~Model();
        int nvert();
        int nface();
        Vec3f vert(int i);
        faceData face(int i);
        Vec3f uv(int i);        
        void PrintVert();
        void PrintFace();
};

#endif //__MODEL_H__