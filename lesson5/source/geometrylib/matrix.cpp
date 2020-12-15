#include "matrix.h"
#include "assert.h"
#include <iostream>

Matrix::Matrix(int r, int c):rows(r), cols(c){
    m = vector<vector<float>>(r, vector<float>(c, 1.f));
}

Matrix::Matrix(Vec3f vec):rows(4), cols(1){
    m = vector<vector<float>>(4, vector<float>(1, 1.f));
    m[0][0] = vec[0];
    m[1][0] = vec[1];
    m[2][0] = vec[2];
}

Matrix::~Matrix(){}

int Matrix::row(){return rows;}

int Matrix::col(){return cols;}

vector<float>& Matrix::operator[](const int row){
    assert(row<rows);
    return m[row];
}

Matrix Matrix::operator*(const Matrix& mul){
    assert(cols == mul.rows);
    Matrix ret(rows, mul.cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<mul.cols; j++){
            ret.m[i][j] = 0.f;
            for(int k=0; k<cols; k++){
                ret.m[i][j] += m[i][k]*mul.m[k][j];
            }
        }
    }
    return ret;
}

Matrix Matrix::operator*(float f){
    Matrix ret(rows, cols);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            ret.m[i][j] = m[i][j]*f;
        }
    }
    return ret;
}

Matrix Matrix::identity(int dimensions){
    Matrix ret(dimensions, dimensions);
    for(int i=0; i<dimensions; i++){
        for(int j=0; j<dimensions; j++){
            ret.m[i][j] = i==j?1.f:0.f;
        }
    }
    return ret;
}


Matrix Matrix::transpose(){
    Matrix ret(cols, rows);
    for(int i=0; i<cols; i++){
        for(int j=0; j<rows; j++){
            ret.m[i][j] = m[j][i];
        }
    }
    return ret;
}

// 求逆矩阵
Matrix Matrix::inverse(){
    assert(rows==cols);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
    Matrix result(rows, cols*2);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            result[i][j] = m[i][j];
    for(int i=0; i<rows; i++)
        result[i][i+cols] = 1;
    // first pass
    for (int i=0; i<rows-1; i++) {
        // normalize the first row
        for(int j=result.cols-1; j>=0; j--)
            result[i][j] /= result[i][i];
        for (int k=i+1; k<rows; k++) {
            float coeff = result[k][i];
            for (int j=0; j<result.cols; j++) {
                result[k][j] -= result[i][j]*coeff;
            }
        }
    }
    // normalize the last row
    for(int j=result.cols-1; j>=rows-1; j--)
        result[rows-1][j] /= result[rows-1][rows-1];
    // second pass
    for (int i=rows-1; i>0; i--) {
        for (int k=i-1; k>=0; k--) {
            float coeff = result[k][i];
            for (int j=0; j<result.cols; j++) {
                result[k][j] -= result[i][j]*coeff;
            }
        }
    }
    // cut the identity matrix back
    Matrix truncate(rows, cols);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            truncate[i][j] = result[i][j+cols];
    return truncate;
}

std::ostream& operator<< (ostream& s, Matrix a){
    for(int i=0; i<a.row(); i++){
        for(int j=0; j<a.col(); j++){
            s << a[i][j]<<"\t";
        }
        s << endl;
    }
    return s;
}