#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
using namespace std;

class Model{
    private:
        vector<Vec3f> verts;
        vector<vector<int>> faces;
    public:
        Model(char * filename);
        ~Model();
        int nvert();
        int nface();
        Vec3f vert(int i);
        vector<int> face(int i);
        void PrintVert();
        void PrintFace();
};

#endif