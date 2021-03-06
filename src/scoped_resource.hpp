/* $Id: scoped_resource.hpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SCOPED_RESOURCE_H_INCLUDED
#define SCOPED_RESOURCE_H_INCLUDED

#include <cstdio> //for FILE

namespace util
{
/**
* A class template, scoped_resource, designed to
* implement the Resource Acquisition Is Initialization (RAII) approach
* to resource management. scoped_resource is designed to be used when
* a resource is initialized at the beginning or middle of a scope,
* and released at the end of the scope. The template argument
* ReleasePolicy is a functor which takes an argument of the
* type of the resource, and releases it.
*
* Usage example, for working with files:
*
* @code
* struct close_file { void operator()(int fd) const {close(fd);} };
* ...
* {
*    const scoped_resource<int,close_file> file(open("file.txt",O_RDONLY));
*    read(file, buf, 1000);
* } // file is automatically closed here
* @endcode
*
* Note that scoped_resource has an explicit constructor, and prohibits
* copy-construction, and thus the initialization syntax, rather than
* the assignment syntax must be used when initializing.
*
* i.e. using scoped_resource<int,close_file> file = open("file.txt",O_RDONLY);
* in the above example is illegal.
*
*/
template<typename T,typename ReleasePolicy>
class scoped_resource
{
	T resource;

	//prohibited operations
	scoped_resource(const scoped_resource&);
	scoped_resource& operator=(const scoped_resource&);
public:
	typedef T resource_type;
	typedef ReleasePolicy release_type;

  /**
  * Constructor
	*
	* @ param res This is the resource to be managed
  */
	scoped_resource(resource_type res = resource_type())
			: resource(res) {}

  /**
  * The destructor is the main point in this class. It takes care of proper
	* deletion of the resource, using the provided release policy.
	*/
	~scoped_resource()
	{
		release_type()(resource);
	}

  /**
  * This operator makes sure you can access and use the scoped_resource
	* just like you were using the resource itself.
  *
  * @ret the underlying resource
	*/
	operator resource_type() const { return resource; }

  /**
  * This function provides explicit access to the resource. Its behaviour
  * is identical to operator resource_type()
	*
	* @ret the underlying resource
  */
	resource_type get() const { return resource; }

	/**
  * This function provides convenient direct access to the -> operator
  * if the underlying resource is a pointer. Only call this function
  * if resource_type is a pointer type.
  */
	resource_type operator->() const { return resource; }

	void assign(const resource_type& o) {
		release_type()(resource);
		resource = o;
	}
};

/**
* A helper policy for scoped_ptr.
* It will call the delete operator on a pointer, and assign the pointer to 0
*/
struct delete_item {
	template<typename T>
	void operator()(T*& p) const { delete p; p = 0; }
};
/**
* A helper policy for scoped_array.
* It will call the delete[] operator on a pointer, and assign the pointer to 0
*/
struct delete_array {
	template<typename T>
	void operator()(T*& p) const { delete [] p; p = 0; }
};

/**
* A class which implements an approximation of
* template<typename T>
* typedef scoped_resource<T*,delete_item> scoped_ptr<T>;
*
* It is a convenient synonym for a common usage of @ref scoped_resource.
* See scoped_resource for more details on how this class behaves.
*
* Usage example:
* @code
* {
*    const scoped_ptr<Object> ptr(new Object);
*    ...use ptr as you would a normal Object*...
* } // ptr is automatically deleted here
* @endcode
*
* NOTE: use this class only to manage a single object, *never* an array.
* Use scoped_array to manage arrays. This distinction is because you
* may call delete only on objects allocated with new, delete[] only
* on objects allocated with new[].
*/
template<typename T>
struct scoped_ptr : public scoped_resource<T*,delete_item>
{
	explicit scoped_ptr(T* p) : scoped_resource<T*,delete_item>(p) {}
};

/**
* This class has identical behaviour to @ref scoped_ptr, except it manages
* heap-allocated arrays instead of heap-allocated single objects
*
* Usage example:
* @code
* {
*    const scoped_array<char> ptr(new char[n]);
*    ...use ptr as you would a normal char*...
* } // ptr is automatically deleted here
* @endcode
*
*/
template<typename T>
struct scoped_array : public scoped_resource<T*,delete_array>
{
	explicit scoped_array(T* p) : scoped_resource<T*,delete_array>(p) {}
};

/**
 * This class specializes the scoped_resource to implement scoped FILEs. Not
 * sure this is the best place to place such an utility, though.
 */
struct close_FILE
{
	void operator()(FILE* f) const { if(f != NULL) { fclose(f); } }
};
typedef scoped_resource<FILE*,close_FILE> scoped_FILE;

}

#endif
