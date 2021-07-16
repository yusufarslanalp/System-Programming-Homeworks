#include <iostream>
#include <vector>
#include <stdexcept>

class Matrix
{

private:

    class RowIterator
    {
    public:
        RowIterator(Matrix* mat, int rowNum) :_mat(mat), _rowNum(rowNum) {}
        double& operator[] (int colNum) { return _mat->_data[_rowNum*_mat->_sizeX + colNum]; }
    private:
        Matrix* _mat;
        int _rowNum;
    };

    int _sizeY, _sizeX;
    std::vector<double> _data;

public:

    Matrix(int sizeY, int sizeX) : _sizeY(sizeY), _sizeX(sizeX), _data(_sizeY*_sizeX){}
    Matrix(std::vector<std::vector<double> > initList) : _sizeY(initList.size()), _sizeX(_sizeY>0 ? initList.begin()->size() : 0), _data()
    { 
        _data.reserve(_sizeY*_sizeX);
        for (const std::vector<double>& list : initList)
        {
            _data.insert(_data.end(), list.begin(), list.end());
        }
    }

    RowIterator operator[] (int rowNum) { return RowIterator(this, rowNum); }

    int getSize() { return _sizeX*_sizeY; }
    int getSizeX() { return _sizeX; }
    int getSizeY() { return _sizeY; }

    Matrix reduce(int rowNum, int colNum)
    {
        Matrix mat(_sizeY-1, _sizeX-1);
        int rowRem = 0;
        for (int y = 0; y < _sizeY; y++)
        {
            if (rowNum == y)
            {
                rowRem = 1;
                continue;
            }
            int colRem = 0;
            for (int x = 0; x < _sizeX; x++)
            {
                if (colNum == x)
                {
                    colRem = 1;
                    continue;
                }
                mat[y - rowRem][x - colRem] = (*this)[y][x];
            }
        }
        return mat;
    }

    Matrix replaceCol(int colNum, std::vector<double> newCol)
    {
        Matrix mat = *this;
        for (int y = 0; y < _sizeY; y++)
        {
            mat[y][colNum] = newCol[y];
        }
        return mat;
    }

};

double solveMatrix(Matrix mat)
{
    if (mat.getSizeX() != mat.getSizeY()) throw std::invalid_argument("Not square matrix");
    if (mat.getSize() > 1)
    {
        double sum = 0.0;
        int sign = 1;
        for (int x = 0; x < mat.getSizeX(); x++)
        {
            sum += sign * mat[0][x] * solveMatrix(mat.reduce(0, x));
            sign = -sign;
        }
        return sum;
    }

    return mat[0][0];
}

std::vector<double> solveEq(std::vector< std::pair<double, double> > points)
{
    int my_size;
    std::vector<std::vector<double> > xes(points.size());
    my_size = points.size();
    for (int i = 0; i< my_size; i++)
    {
        xes[i].push_back(1);
        for (int j = 1; j< my_size; j++)
        {
            xes[i].push_back(xes[i].back() * points[i].first);
        }
    }

    Matrix mat(xes);

    std::vector<double> ys( my_size );

    for (int i = 0; i < my_size; i++)
    {
        ys[i] = points[i].second;
    }

    double w = solveMatrix(mat);

    std::vector<double> result(my_size, 0.0);

    if(w!=0)
        for (int i = 0; i < (int)ys.size(); i++)
        {
            result[i] = solveMatrix(mat.replaceCol(i, ys));
            result[i] /= w;
        }

    return result;
}

void printCoe(std::vector<double> coe)
{
    int size = coe.size();
    for (int i = 0; i < size; i++)
    {
        std::cout << coe[i] << std::endl;
    }
    std::cout << std::endl;
}

void
find_coeffs( float coeffs[], float datax[], float datay[], int size )
{
    std::vector< std::pair<double, double> > points1;
    for( int i = 0; i < size; i++ )
    {
    	points1.push_back( {datax[i], datay[i]} );
    }
    std::vector<double> vec_coe = solveEq(points1);
    size = vec_coe.size();
    for (int i = 0; i < size; i++)
    {
        coeffs[i] = vec_coe[i];
    }
}
