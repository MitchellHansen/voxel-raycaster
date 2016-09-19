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

#ifndef SFML_VECTOR4_H
#define SFML_VECTOR4_H

namespace sf {
	////////////////////////////////////////////////////////////
	/// \brief Utility template class for manipulating
	///        2-dimensional vectors
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	class Vector4 {
	public:

		////////////////////////////////////////////////////////////
		/// \brief Default constructor
		///
		/// Creates a Vector4(0, 0).
		///
		////////////////////////////////////////////////////////////
		Vector4();

		////////////////////////////////////////////////////////////
		/// \brief Construct the vector from its coordinates
		///
		/// \param X X coordinate
		/// \param Y Y coordinate
		///
		////////////////////////////////////////////////////////////
		Vector4(T X, T Y, T Z, T W);

		////////////////////////////////////////////////////////////
		/// \brief Construct the vector from another type of vector
		///
		/// This constructor doesn't replace the copy constructor,
		/// it's called only when U != T.
		/// A call to this constructor will fail to compile if U
		/// is not convertible to T.
		///
		/// \param vector Vector to convert
		///
		////////////////////////////////////////////////////////////
		template <typename U>
		explicit Vector4(const Vector4<U>& vector);

		////////////////////////////////////////////////////////////
		// Member data
		////////////////////////////////////////////////////////////
		T x; ///< X coordinate of the vector
		T y; ///< Y coordinate of the vector
		T z; ///< Z coordinate of the vector
		T w; ///< W coordinate of the vector
	};

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of unary operator -
	///
	/// \param right Vector to negate
	///
	/// \return Memberwise opposite of the vector
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator -(const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator +=
	///
	/// This operator performs a memberwise addition of both vectors,
	/// and assigns the result to \a left.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return Reference to \a left
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T>& operator +=(Vector4<T>& left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator -=
	///
	/// This operator performs a memberwise subtraction of both vectors,
	/// and assigns the result to \a left.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return Reference to \a left
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T>& operator -=(Vector4<T>& left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator +
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return Memberwise addition of both vectors
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator +(const Vector4<T>& left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator -
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return Memberwise subtraction of both vectors
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator -(const Vector4<T>& left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator *
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a scalar value)
	///
	/// \return Memberwise multiplication by \a right
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator *(const Vector4<T>& left, T right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator *
	///
	/// \param left  Left operand (a scalar value)
	/// \param right Right operand (a vector)
	///
	/// \return Memberwise multiplication by \a left
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator *(T left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator *=
	///
	/// This operator performs a memberwise multiplication by \a right,
	/// and assigns the result to \a left.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a scalar value)
	///
	/// \return Reference to \a left
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T>& operator *=(Vector4<T>& left, T right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator /
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a scalar value)
	///
	/// \return Memberwise division by \a right
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T> operator /(const Vector4<T>& left, T right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator /=
	///
	/// This operator performs a memberwise division by \a right,
	/// and assigns the result to \a left.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a scalar value)
	///
	/// \return Reference to \a left
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	Vector4<T>& operator /=(Vector4<T>& left, T right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator ==
	///
	/// This operator compares strict equality between two vectors.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return True if \a left is equal to \a right
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	bool operator ==(const Vector4<T>& left, const Vector4<T>& right);

	////////////////////////////////////////////////////////////
	/// \relates Vector4
	/// \brief Overload of binary operator !=
	///
	/// This operator compares strict difference between two vectors.
	///
	/// \param left  Left operand (a vector)
	/// \param right Right operand (a vector)
	///
	/// \return True if \a left is not equal to \a right
	///
	////////////////////////////////////////////////////////////
	template <typename T>
	bool operator !=(const Vector4<T>& left, const Vector4<T>& right);



	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>::Vector4() :
		x(0),
		y(0),
		z(0),
		w(0){

	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>::Vector4(T X, T Y, T Z, T W) :
		x(X),
		y(Y),
		z(Z),
		w(W) {

	}


	////////////////////////////////////////////////////////////
	template <typename T>
	template <typename U>
	inline Vector4<T>::Vector4(const Vector4<U>& vector) :
		x(static_cast<T>(vector.x)),
		y(static_cast<T>(vector.y)),
		z(static_cast<T>(vector.z)), 
		w(static_cast<T>(vector.w)) {
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator -(const Vector4<T>& right) {
		return Vector4<T>(
			-right.x, 
			-right.y, 
			-right.z, 
			-right.w
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>& operator +=(Vector4<T>& left, const Vector4<T>& right) {
		left.x += right.x;
		left.y += right.y;
		left.z += right.z;
		left.w += right.w;

		return left;
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>& operator -=(Vector4<T>& left, const Vector4<T>& right) {
		left.x -= right.x;
		left.y -= right.y;
		left.z -= right.z;
		left.w -= right.w;

		return left;
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator +(const Vector4<T>& left, const Vector4<T>& right) {
		return Vector4<T>(
			left.x + right.x,
			left.y + right.y,
			left.z + right.z,
			left.w + right.w
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator -(const Vector4<T>& left, const Vector4<T>& right) {
		return Vector4<T>(
			left.x - right.x,
			left.y - right.y,
			left.z - right.z,
			left.w - right.w
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator *(const Vector4<T>& left, T right) {
		return Vector4<T>(
			left.x * right, 
			left.y * right, 
			left.z * right, 
			left.w * right
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator *(T left, const Vector4<T>& right) {
		return Vector4<T>(
			right.x * left, 
			right.y * left, 
			right.z * left, 
			right.w * left
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>& operator *=(Vector4<T>& left, T right) {
		left.x *= right;
		left.y *= right;
		left.z *= right;
		left.w *= right;

		return left;
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T> operator /(const Vector4<T>& left, T right) {
		return Vector4<T>(
			left.x / right,
			left.y / right,
			left.z / right, 
			left.w / right
			);
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline Vector4<T>& operator /=(Vector4<T>& left, T right) {
		left.x /= right;
		left.y /= right;
		left.z /= right;
		left.w /= right;

		return left;
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline bool operator ==(const Vector4<T>& left, const Vector4<T>& right) {
		return 
			(left.x == right.x) && 
			(left.y == right.y) && 
			(left.z == right.z) && 
			(left.w == right.w);;
	}


	////////////////////////////////////////////////////////////
	template <typename T>
	inline bool operator !=(const Vector4<T>& left, const Vector4<T>& right) {
		return 
			(left.x != right.x) || 
			(left.y != right.y) || 
			(left.z != right.z) ||
			(left.w != right.w);
	}

	// Define the most common types
	typedef Vector4<int>          Vector4i;
	typedef Vector4<unsigned int> Vector4u;
	typedef Vector4<float>        Vector4f;

} // namespace sf


#endif // SFML_Vector4_HPP


  ////////////////////////////////////////////////////////////
  /// \class sf::Vector4
  /// \ingroup system
  ///
  /// sf::Vector4 is a simple class that defines a mathematical
  /// vector with two coordinates (x and y). It can be used to
  /// represent anything that has two dimensions: a size, a point,
  /// a velocity, etc.
  ///
  /// The template parameter T is the type of the coordinates. It
  /// can be any type that supports arithmetic operations (+, -, /, *)
  /// and comparisons (==, !=), for example int or float.
  ///
  /// You generally don't have to care about the templated form (sf::Vector4<T>),
  /// the most common specializations have special typedefs:
  /// \li sf::Vector4<float> is sf::Vector4f
  /// \li sf::Vector4<int> is sf::Vector4i
  /// \li sf::Vector4<unsigned int> is sf::Vector4u
  ///
  /// The sf::Vector4 class has a small and simple interface, its x and y members
  /// can be accessed directly (there are no accessors like setX(), getX()) and it
  /// contains no mathematical function like dot product, cross product, length, etc.
  ///
  /// Usage example:
  /// \code
  /// sf::Vector4f v1(16.5f, 24.f);
  /// v1.x = 18.2f;
  /// float y = v1.y;
  ///
  /// sf::Vector4f v2 = v1 * 5.f;
  /// sf::Vector4
  /// v3 = v1 + v2;
  ///
  /// bool different = (v2 != v3);
  /// \endcode
  ///
  /// Note: for 3-dimensional vectors, see sf::Vector3.
  ///
  ////////////////////////////////////////////////////////////