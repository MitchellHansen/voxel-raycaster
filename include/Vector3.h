////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2016 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef GAME_VECTOR3_H
#define GAME_VECTOR3_H

template <typename T>
class Vector3
{
public:
    
    // Default constructor
    // Creates a Vector3(0, 0, 0).
    Vector3();


    // Construct the vector from its coordinates
    Vector3(T X, T Y, T Z);

    
    // Construct the vector from another type of vector
    // This constructor doesn't replace the copy constructor,
    // it's called only when U != T.
    // A call to this constructor will fail to compile if U
    // is not convertible to T.
    template <typename U>
    explicit Vector3(const Vector3<U>& vector);

    // Member data
    T x;
    T y;
    T z;
};

    // Vector3
    // Overload of unary operator -
    // left Vector to negate
    // Memberwise opposite of the vector
    template <typename T>
    Vector3<T> operator -(const Vector3<T>& left);

    // Overload of binary operator +=
    // This operator performs a memberwise addition of both vectors,
    // and assigns the result to left.
    // returns Reference to left
    template <typename T>
    Vector3<T>& operator +=(Vector3<T>& left, const Vector3<T>& right);

    // Overload of binary operator -=
    // This operator performs a memberwise subtraction of both vectors,
    // and assigns the result to left.
    // returns Reference to left
    template <typename T>
    Vector3<T>& operator -=(Vector3<T>& left, const Vector3<T>& right);

    // Overload of binary operator +
    // returns Memberwise addition of both vectors
    template <typename T>
    Vector3<T> operator +(const Vector3<T>& left, const Vector3<T>& right);
    // Overload of binary operator -
    // returns Memberwise subtraction of both vectors
    template <typename T>
    Vector3<T> operator -(const Vector3<T>& left, const Vector3<T>& right);
    
    // Overload of binary operator *
    // returns Memberwise multiplication by right
    template <typename T>
    Vector3<T> operator *(const Vector3<T>& left, T right);
    
    // Overload of binary operator *
    // returns Memberwise multiplication by left
    template <typename T>
    Vector3<T> operator *(T left, const Vector3<T>& right);
    
    // Overload of binary operator *=
    // This operator performs a memberwise multiplication by right,
    // and assigns the result to left.
    // returns Reference to left
    template <typename T>
    Vector3<T>& operator *=(Vector3<T>& left, T right);
    
    // Overload of binary operator /
    // returns Memberwise division by right
    template <typename T>
    Vector3<T> operator /(const Vector3<T>& left, T right);
    
    // Overload of binary operator /=
    // This operator performs a memberwise division by right,
    // and assigns the result to left.
    // returns Reference to left
    template <typename T>
    Vector3<T>& operator /=(Vector3<T>& left, T right);
    
    // Overload of binary operator ==
    // This operator compares strict equality between two vectors.
    // returns True if left is equal to right
    template <typename T>
    bool operator ==(const Vector3<T>& left, const Vector3<T>& right);
    
    // Overload of binary operator !=
    // This operator compares strict difference between two vectors.
    // returns True if  left is not equal to  right
    template <typename T>
    bool operator !=(const Vector3<T>& left, const Vector3<T>& right);

#include <SFML/System/Vector3.inl>

    // Define the most common types
    typedef Vector3<int>   Vector3i;
    typedef Vector3<float> Vector3f;



#endif 




