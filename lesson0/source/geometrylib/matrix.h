#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
using namespace std;
const int default_row = 4;
class Matrix{
public:
    Matrix(int r=default_row, int c=default_row);
    ~Matrix();
    vector<float> & operator[](const int row);
    Matrix operator * (const Matrix &mul);
    Matrix operator * (float f);
    static Matrix identity(int dimensions);
    Matrix transpose();
    Matrix inverse();
    int row();
    int col();

    friend ostream& operator<<(ostream& s, Matrix& m);
protected:
    int rows;
    int cols;
    vector<vector<float>> m;
};

#endif //__MATRIX_H__