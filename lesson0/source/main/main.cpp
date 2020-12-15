#include "tgaimage.h"
#include "model.h"
#include "matrix.h"
#include "geometry.h"
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

const TGAColor red = TGAColor(0, 0, 255, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(255, 0, 0, 255);

const int width = 1024;
const int height = 1024;
const int depth = 255; //假如用纹理存储深度，最大一般都是到255

struct pointAndUv{
        Vec3f point[3];
        Vec3f uvs[3];
    };

Model *model = NULL;
int *zbuffer = NULL;
Vec3f light_dir(0,0,-1);
Vec3f center(0,0,0);
Vec3f camera(1,1,3);
Vec3f up(0,1,0);

Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

// 返回ModelView矩阵 z正方向是摄像机只想模型中心点
Matrix v2m(Vec3f camera, Vec3f center, Vec3f up) {
    Vec3f z = (center-camera).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix m = Matrix::identity(4);
    for(int i = 0; i < 3; i++){
        m[0][i] = x[i];
        m[1][i] = y[i];
        m[2][i] = z[i];
        m[i][3] = -center[i];
    }
    return m;
}

// viewport(width/8, height/8, width*3/4, height*3/4);
// 这里的viewport是view矩阵，把[-1,1]*[-1,1]*[-1,1]的坐标变换到[x,x+w]*[y,y+h]*[0,d]的立方体中。渲染管线一般把x，y定为0
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

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

void line(Vec2i p1, Vec2i p2, TGAImage &image, TGAColor color){
    line(p1.x, p1.y, p2.x, p2.y, image, color);
}

// 后面的优化是对float，以及运算的优化（目前没看太懂）

void triangle(Vec2i p1, Vec2i p2, Vec2i p3, TGAImage &image, TGAColor color){
    line(p1, p2, image, color);
    line(p2, p3, image, color);
    line(p3, p1, image, color);
}

// 扫描线填充算法
// 先将顶点按y升序排列，在取中间，分割成两个三角形，从下向上，逐行扫描填充
void fill_triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
    if (t0.y>t1.y) std::swap(t0, t1); 
    if (t0.y>t2.y) std::swap(t0, t2); 
    if (t1.y>t2.y) std::swap(t1, t2); 
    int total_height = t2.y-t0.y; 
    for (int y=t0.y; y<=t1.y; y++) { 
        int segment_height = t1.y-t0.y+1; 
        float alpha = (float)(y-t0.y)/total_height; 
        float beta  = (float)(y-t0.y)/segment_height; // be careful with divisions by zero 
        Vec2i A = t0 + (t2-t0)*alpha; 
        Vec2i B = t0 + (t1-t0)*beta; 
        if (A.x>B.x) std::swap(A, B); 
        for (int j=A.x; j<=B.x; j++) { 
            image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
        } 
    } 
    for (int y=t1.y; y<=t2.y; y++) { 
        int segment_height =  t2.y-t1.y+1; 
        float alpha = (float)(y-t0.y)/total_height; 
        float beta  = (float)(y-t1.y)/segment_height; // be careful with divisions by zero 
        Vec2i A = t0 + (t2-t0)*alpha; 
        Vec2i B = t1 + (t2-t1)*beta; 
        if (A.x>B.x) std::swap(A, B); 
        for (int j=A.x; j<=B.x; j++) { 
            image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
        } 
    } 
}

Vec3f cross(const Vec3f &v1, const Vec3f &v2){
    return Vec3f(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x);
};

// 2d 求点的重心坐标
Vec3f barycentric(Vec3f *pts, Vec2i P) { 
    Vec3f u = cross(Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x), Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.x)>=1) 
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 

    return Vec3f(-1,1,1);
}

// 
void triangle(Vec3f *pts, TGAImage &image, float* zbuffer, TGAColor color) { 
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2f bboxmax(0, 0); 
    Vec2f clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        for (int j=0; j<2; j++) { 
            bboxmin[j] = max(0.f,        min(bboxmin[j], pts[i][j])); 
            bboxmax[j] = min(clamp[j], max(bboxmax[j], pts[i][j])); 
        } 
    }// 求得矩形包围盒 
    Vec2i P; 
    // 遍历包围盒所有的点，重心坐标系满足三参数x y z 都>=0的，就是三角形内的点
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            float z = 0.f;
            for(int i=0; i<3; i++){
                z += (pts[i][2]*bc_screen[i]);
            }
            if(z > zbuffer[P.x*width+P.y]){
                zbuffer[P.x*width+P.y] = z;
                image.set(P.x, P.y, color); 
            }
        } 
    } 
} 

void triangle(pointAndUv pAU, TGAImage &image, TGAImage *texture, float* zbuffer, float intensity) { 
// void triangle(pointAndUv pAU, TGAImage &image, float* zbuffer, TGAColor color) { 
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2f bboxmax(0, 0); 
    Vec2f clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        for (int j=0; j<2; j++) { 
            bboxmin[j] = max(0.f,        min(bboxmin[j], pAU.point[i][j])); 
            bboxmax[j] = min(clamp[j], max(bboxmax[j], pAU.point[i][j])); 
        } 
    }// 求得矩形包围盒 
    Vec2i P; 
    TGAColor texCol;
    // 遍历包围盒所有的点，重心坐标系满足三参数x y z 都>=0的，就是三角形内的点
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pAU.point, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            int z = 0;
            for(int i=0; i<3; i++){
                z += ((pAU.point[i][2]*bc_screen[i])*depth);
            }
            if(z > zbuffer[P.x*width+P.y]){ // z正方向是朝视点
                zbuffer[P.x*width+P.y] = z;
                float u=0.f, v=0.f;
                for(int i=0; i<3; i++){ // 数据插值的核心-重心坐标系（1-u-v, u, v）,该比例能运用在所有的顶点数据
                    u += (pAU.uvs[i][0]*bc_screen[i]);
                    v += (pAU.uvs[i][1]*bc_screen[i]);
                }
                float floatU = texture->get_width() * u;
                float floatV = texture->get_height() * (1-v);
                // cerr << u << " v:" << v << endl;
                texCol = texture->get(floatU, floatV);
                image.set(P.x, P.y, texCol*intensity); 
                // image.set(P.x, P.y, color);
            }
        } 
    } 
} 

Vec3f worldToScreen(Vec3f point){
    Vec3f ret;
    // 假设视点在(0，0，2)处，这里的视点坐标是基于模型空间的，模型空间是取值范围-1，1
    float temp = 1-point.z/2;
    ret.x = (int)(((point.x+1.)/2.*width+.5)/temp);
    ret.y = (int)(((point.y+1.)/2.*height+.5)/temp);
    ret.z = point.z/temp;
    return ret;
}



int main(int argc, char** argv){
    //TGAImage image(width, height, TGAImage::RGBA);
    // triangle(t0[0], t0[1], t0[2], image, red);
    // triangle(t1[0], t1[1], t1[2], image, green);
    // triangle(t2[0], t2[1], t2[2], image, blue);
    // fill_triangle(t0[0], t0[1], t0[2], image, red);
    // fill_triangle(t1[0], t1[1], t1[2], image, green);
    // fill_triangle(t2[0], t2[1], t2[2], image, blue);

    // Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    // Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    // triangle(t0, image, red);
    // triangle(t1, image, green);
    // triangle(t2, image, blue);
    // image.flip_vertically();
    // image.write_tga_file("out_triangle.tga");

    // // draw model 扫描算法 start
    // Model *model = NULL;
    // if(argc==2){
    //     model = new Model(argv[1]);
    // }else{
    //     model = new Model("obj/african_head");
    // }
    // TGAImage image(width, height, TGAImage::RGBA);
    // for(int i=0; i<model->nface(); i++){
    //     vector<int> faceData = model->face(i);
    //     for(int j=0; j<3; j++){
    //         Vec3f point1 = model->vert(faceData[j]);
    //         Vec3f point2 = model->vert(faceData[(j+1)%3]);
    //         int x0 = (point1.x+1.0)/2.*width;
    //         int y0 = (point1.y+1.0)/2.*height;
    //         int x1 = (point2.x+1.0)/2.*width;
    //         int y1 = (point2.y+1.0)/2.*height;
    //         //cerr << "(" << x0 << "," << y0 << "), " << "(" << x1 << "," << y1 << ")" << endl;
    //         line(x0, y0, x1, y1, image, green);
    //     }
    // }
    // image.flip_vertically();
    // image.write_tga_file("out_african_head.tga");   
    // // draw model end

    // draw model 重心坐标系 start
    // Model *model = NULL;
    // if(argc==2){
    //     model = new Model(argv[1]);
    // }
    // else{
    //     model = new Model("obj/african_head");
    // }

    // TGAImage image(width, height, TGAImage::RGB);
    // Vec2i point[3] = {Vec2i(0, 0),   Vec2i(0, 0),  Vec2i(0, 0)};;
    // for(int i=0; i<model->nface(); i++){
    //     vector<int> faceData = model->face(i);
    //     for(int j=0; j<3; j++){
    //         Vec3f rawData = model->vert(faceData[j]);
    //         point[j].x = (int)((rawData.x+1.)/2.*width);
    //         point[j].y = (int)((rawData.y+1.)/2.*height);
    //     }
    //     triangle(point, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    // }
    // image.flip_vertically();
    // image.write_tga_file("out_model_triangle_barycentric.tga");
    // draw model 重心坐标系 end

    Model *model = NULL;
    TGAImage *texture = NULL;
    float *zbuffer = new float[width*height];
    if(argc==3){
        model = new Model(argv[1]);
        texture = new TGAImage();
        texture->read_tga_file(argv[2]);
    }else{
        model = new Model("obj/african_head");
    }

    TGAImage image(width, height, TGAImage::RGB);

    {   
        // 投影矩阵
        Matrix ModelView = v2m(camera, center, up);
        Matrix Projection = Matrix::identity(4);
        Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
        Projection[3][2] = -1.f/(camera-center).norm();

        pointAndUv pAU;
        Vec3f lightDir = Vec3f(0,0,-1);
        lightDir.normalize();

        for(int i=0; i<width*height; i++){
            zbuffer[i] = -numeric_limits<float>::max();
        }

        Matrix mvp = ViewPort*Projection*ModelView;

        for(int i=0; i<model->nface(); i++){
            faceData fData = model->face(i);
            Vec3f triangleData[3];
            for(int j=0; j<3; j++) {
                Vec3f pointRawData = model->vert(fData.vertIndice[j]);
                triangleData[j] = pointRawData;

                pAU.point[j] = m2v(mvp * Matrix(pointRawData));
                // pAU.point[j] = worldToScreen(pointRawData);
                // cout << pAU.point[j].x << " " << pAU.point[j].y << " " << pAU.point[j].z << endl;

                pAU.uvs[j] = model->uv(fData.uvIndices[j]);
            }
            Vec3f normal = (triangleData[2]-triangleData[0]) ^ (triangleData[1]-triangleData[0]);
            normal.normalize();
            float intensity = normal*lightDir;
            if (intensity >0 ){
                triangle(pAU, image, texture, zbuffer, intensity);
                // triangle(pAU, image, zbuffer, TGAColor(rand()%255, rand()%255, rand()%255, 255));
            }
            // triangle(point, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
        }
        image.flip_vertically();
        image.write_tga_file("out_model_mvp.tga");
    }

    {
        TGAImage depth_image(width, height, TGAImage::RGB);
        int gray = 0;
        TGAColor grayColor;
        for(int i=0; i<width; i++){
            for(int j=0; j<height; j++){
                gray = floor(zbuffer[i+j*width]);
                depth_image.set(i, j, TGAColor(gray, gray, gray, 255));
            }     
        }
        depth_image.flip_vertically();
        depth_image.write_tga_file("out_zbuffer.tga");
    }

    delete model;
    if(texture!=NULL) delete texture;
    delete [] zbuffer;

    // 画个矩形
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

