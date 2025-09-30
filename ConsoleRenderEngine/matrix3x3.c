#include "matrix3x3.h"

Vector3 matrix3x3_mul_vector3(Vector3 a, Matrix3x3 M)
{
    Vector3 result;
    result.x = M.m[0][0] * a.x + M.m[0][1] * a.y + M.m[0][2] * a.z;
    result.y = M.m[1][0] * a.x + M.m[1][1] * a.y + M.m[1][2] * a.z;
    result.z = M.m[2][0] * a.x + M.m[2][1] * a.y + M.m[2][2] * a.z;
    return result;
}

Matrix3x3 matrix3x3_transpose(Matrix3x3 M)
{
    Matrix3x3 transposed;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            transposed.m[i][j] = M.m[j][i];
        }
    }

    return transposed;
}

Matrix3x3 matrix3x3_mul(Matrix3x3 M1, Matrix3x3 M2)
{
    Matrix3x3 mult;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            mult.m[i][j] = 0.0;
            for (int k = 0; k < 3; k++)
            {
                mult.m[i][j] += M1.m[i][k] * M2.m[k][j];
            }
        }
    }

    return mult;
}


