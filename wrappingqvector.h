#ifndef WRAPPINGQVECTOR_H
#define WRAPPINGQVECTOR_H

#include <QVector>

template <typename T> class WrappingQVector
        : public QVector<T>
{
public:
    WrappingQVector(int maxSize);

    void push(QVector<T> in);
    void push(T in);
    void insert(QVector<T> in);
    void clear();
    int maxSize() const;
    void setMaxSize(int maxSize);
private:
    int m_maxSize;
    int pos;
};

template<typename T>
WrappingQVector<T>::WrappingQVector(int maxSize)
{
    m_maxSize = maxSize;
    pos = 0;
}

template<typename T>
void WrappingQVector<T>::push(QVector<T> in)
{
    for (int i = 0; i < in.size(); i++)
    {
        if (in.size() < maxSize() && pos == 0)
        {
            in.push_back(in[i]);
        }
        else if (in.size() <= maxSize())
        {
            (*this)[pos] = in[i];
            pos++;
            if (pos >= in.size())
            {
                pos = 0;
            }
        }
        else
        {
            (*this)[pos] = in[i];
            pos++;
            if (pos + in.size() - maxSize() <= in.size())
            {
                in.erase(in.begin() + pos, in.begin() + pos + in.size() - maxSize());
            }
            else
            {
                in.erase(in.begin() + pos, in.end());
            }
            if (pos >= in.size())
            {
                pos = 0;
            }
        }
    }
}

template<typename T>
void WrappingQVector<T>::push(T in)
{
    if (this->size() < maxSize() && pos == 0)
    {
        this->push_back(in);
    }
    else if (this->size() <= maxSize())
    {
        (*this)[pos] = in;
        pos++;
        if (pos >= this->size())
        {
            pos = 0;
        }
    }
    else
    {
        (*this)[pos] = in;
        pos++;
        if (pos + this->size() - maxSize() <= this->size())
        {
            this->erase(this->begin() + pos, this->begin() + pos + this->size() - maxSize());
        }
        else
        {
            this->erase(this->begin() + pos, this->end());
        }
        if (pos >= this->size())
        {
            pos = 0;
        }
    }
}

template<typename T>
void WrappingQVector<T>::clear()
{
    QVector<T>::clear();
    pos = 0;
}

template<typename T>
int WrappingQVector<T>::maxSize() const
{
    return m_maxSize;
}

template<typename T>
void WrappingQVector<T>::setMaxSize(int maxSize)
{
    m_maxSize = maxSize;
}

#endif // WRAPPINGQVECTOR_H
