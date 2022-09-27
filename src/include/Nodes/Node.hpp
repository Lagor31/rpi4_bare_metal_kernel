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

#ifndef __SD__NODES__NODE__
#define __SD__NODES__NODE__

#include "NodeInterface.hpp"

namespace SD::Nodes {
    /**
     * @class   Node
     * @brief   Basic Linked List node
     *
     * The Linked List node doesn't do much beyond carry a value, but it serves
     * as the base for all other node types.
     */
    template<typename T>
    class Node : public NodeInterface<T>
    {
    public:
        /**
         * Constructor initializes the value of the node
         * @param value [description]
         */
        Node(T value) : value(value)
        {
            // empty constructor
        }

        /**
         * Destructor should clean up memory
         */
        ~Node()
        {
            // nothing to clean up
        }

        /**
         * Retrieves the value contained within the node
         * @return node's value
         */
        T getValue()
        {
            return this->value;
        }

        /**
         * Sets the value contained within the node
         * @param   value value to carry
         * @return *this
         */
        Node<T>& setValue(T value)
        {
            this->value = value;
            return *this;
        }

    protected:
       T value;
    };
}

#endif
