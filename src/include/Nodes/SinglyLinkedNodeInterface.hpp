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

#ifndef __SD__NODES__SINGLY_LINKED_NODE_INTERFACE__
#define __SD__NODES__SINGLY_LINKED_NODE_INTERFACE__

#include "NodeInterface.hpp"

namespace SD::Nodes {

    /**
     * @class   SinglyLinkedNodeInterface
     * @brief   the interface to which all Singly Linked Nodes must ahdere
     *
     * This is an interface for a simple Singly Linked List's Node class. It
     * exists to ensure that the Singly Linked Nodes are interchangeable.
     */
    template<typename T>
    class SinglyLinkedNodeInterface : public NodeInterface<T>
    {
    public:
        /**
         * Gets the next node in the chain
         * @return SinglyLinkedNode&
         */
        NodeInterface<T>* getNextNode();

        /**
         * Sets the next node in the chain
         * @param node Node to set as "next", or NULL to clear
         */
        NodeInterface<T>& setNextNode(NodeInterface<T>* node);
    };
}

#endif
