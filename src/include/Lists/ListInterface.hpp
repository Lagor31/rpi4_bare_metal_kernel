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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#ifndef __SD__LISTS__LIST_INTERFACE__
#define __SD__LISTS__LIST_INTERFACE__

namespace SD::Lists {
    /**
     * @class   ListInterface
     * @brief   The basic list interface that all lists will inherit from.
     *
     * The ListInterface class provides a contract for how lists should
     * behave. This includes the creation, reading, updating, and deletion
     * of nodes.
     */
    template<typename T>
    class ListInterface
    {
    public:
        /**
         * Inserts a new value as the last index on the list.
         *
         * @brief   inserts a new value into the list
         * @param   value
         * @return  *this
         */
        ListInterface<T>& insert(T value);

        /**
         * Inserts a new value at the requested index.
         *
         * @brief   inserts a new value into the list
         * @param   value
         * @param   index
         * @return  *this
         */
        ListInterface<T>& insert(T value, unsigned int index);

        /**
         * @brief   Retrieves a value from the list
         * @param   index   index of the value to retrieve
         * @return  value stored at the index
         */
        T get(unsigned int index);

        /**
         * @brief   Updates the value at an index
         * @param   index
         * @param   value
         * @return  *this
         */
        ListInterface<T>& set(unsigned int index, T value);

        /**
         * @brief Removes nodes from the linked list
         * @param  index    index of the first node to remove
         * @param  count    number of nodes to remove
         * @return *this
         */
        ListInterface<T>& remove(unsigned int index, unsigned int count = 1);

        /**
         * @brief   Counts the number of nodes in the linked list
         * @return  number ofnodes in the linked list
         */
        unsigned int count();
    };
}

#endif
