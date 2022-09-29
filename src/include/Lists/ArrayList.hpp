/**
 * MIT License
 *
 * Copyright (c) 2017 Steven Jimenez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author      Steven Jimenez
 * @copyright   2017 Steven Jimenez
 * @license     MIT
 */

#ifndef __SD__LISTS__ARRAY_LIST__
#define __SD__LISTS__ARRAY_LIST__

//#include <stdexcept>
//#include <iostream>

#include "../Console.h"
#include "ArrayListInterface.hpp"

namespace SD::Lists {

/**
 * @class   ArrayList
 * @brief   Implementation of a simple Array List class
 *
 * This is an implementation of a simple Array List class. This code is
 * intended for demonstrative purposes and is not meant to be used in
 * production-ready code.
 */
template <typename T>
class ArrayList : ArrayListInterface<T> {
 public:
  /**
   * Creates an array list data structure
   */
  ArrayList() : logicalLength(0), physicalLength(1) {
    this->array = new T[this->physicalLength];
  }

  /**
   * Tears down the array list data structure
   */
  ~ArrayList() {
    // empty destructor
  }

  /**
   * Inserts a new value at the requested index. If no index is provided, then
   * assumes that the value should be inserted at the end of the data
   * structure.
   *
   * @param  value
   *
   * @return *this
   */
  ArrayList<T>& insert(T value) {
    return this->insert(value, this->logicalLength);
  }

  /**
   * Inserts a new value at the requested index. If no index is provided, then
   * assumes that the value should be inserted at the end of the data
   * structure.
   *
   * @param  value
   * @param  index
   *
   * @return *this
   */
  ArrayList<T>& insert(T value, unsigned int index) {
    if (index > this->logicalLength) {
      Console::print("Expected index to be within length of list");
    }

    if (1 + this->logicalLength >= this->physicalLength) {
      this->extendArray();
    }

    for (unsigned int i = this->logicalLength + 1; i > index; --i) {
      this->array[i] = this->array[i - 1];
    }

    this->array[index] = value;
    ++this->logicalLength;

    return *this;
  }

  /**
   * Retrieves a value from the identified index
   *
   * @param  index
   * @return value stored at the index
   */
  T get(unsigned int index) {
    if (index >= this->logicalLength) {
      Console::print("Expected index to be within length of list");
    }

    return this->array[index];
  }

  /**
   * Updates a value at the identified index
   *
   * @param  index
   * @param  value
   *
   * @return *this
   */
  ArrayList<T>& set(unsigned int index, T value) {
    if (index >= this->logicalLength) {
      Console::print("Expected index to be within length of list");
    }

    this->array[index] = value;

    return *this;
  }

  /**
   * Removes "count" elements, starting from the identified index
   *
   * @param  index
   * @param  count
   *
   * @return *this
   */
  ArrayList<T>& remove(unsigned int index, unsigned int count = 1) {
    if (index + count > this->logicalLength) {
      Console::print("Expected segment to be within length of list");
    }

    for (unsigned int i = index; i < this->logicalLength; ++i) {
      this->array[i] = this->array[i + count];
    }

    this->logicalLength -= count;
    return *this;
  }

  /**
   * Retrieves the logical length of the array list
   *
   * @return  number of elements in the logical array
   */
  unsigned int count() { return this->logicalLength; }

 protected:
  // the wrapped element
  T* array;
  // the length of the array, as far as the external code is concerned
  unsigned int logicalLength;
  // the length of the array, as far as the runtime execution is concerned
  unsigned int physicalLength;

  /**
   * Doubles the size of the physical array to make room for the logical
   * array
   */
  ArrayList<T>& extendArray() {
    // double the size of the array
    this->physicalLength <<= 1;

    T* array = new T[this->physicalLength];
    for (unsigned int i = 0; i < this->logicalLength; ++i) {
      array[i] = this->array[i];
    }
    this->array = array;

    return *this;
  }
};
}  // namespace SD::Lists

#endif
