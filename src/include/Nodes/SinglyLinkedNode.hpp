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

#ifndef __SD__NODES__SINGLY_LINKED_NODE__
#define __SD__NODES__SINGLY_LINKED_NODE__

#include "Node.hpp"
#include "SinglyLinkedNodeInterface.hpp"

namespace SD::Nodes {

/**
 * @class   SinglyLinkedNode
 * @brief   Implementation of a simple Singly Linked List's Node class
 *
 * This is an implementation of a simple Singly Linked List's Node class.
 * This class is responsible for retrieving values (inherited from Node)
 * and for tracking the next node in the chain.
 */
template <typename T>
class SinglyLinkedNode : public SinglyLinkedNodeInterface<T>, public Node<T> {
 public:
  /**
   * SinglyLinkedNode Constructor
   * @param value   Value to hold within the node
   */
  SinglyLinkedNode(T value) : Node<T>(value), nextNode(NULL) {
    // empty constructor
  }

  ~SinglyLinkedNode() { this->nextNode = NULL; }

  /**
   * Gets the next node in the chain
   * @return SinglyLinkedNode&
   */
  SinglyLinkedNode<T>* getNextNode() { return this->nextNode; }

  /**
   * Sets the next node in the chain
   * @param node Node to set as "next", or NULL to clear
   */
  SinglyLinkedNode<T>& setNextNode(SinglyLinkedNode<T>* node) {
    this->nextNode = node;
    return *this;
  }

  /**
   * Retrieves the value contained within the node
   * @return value contained within node
   */
  T getValue() { return Node<T>::getValue(); }

  /**
   * Sets the value contained within the node
   * @param   value value to carry
   * @return  *this
   */
  SinglyLinkedNode<T>& setValue(T value) {
    Node<T>::setValue(value);
    return *this;
  }

 protected:
  /**
   * @brief   the next node in the chain
   *
   * This is a pointer to the next node in the chain. If no such node
   * exists, then this will be NULL.
   */
  SinglyLinkedNode<T>* nextNode;
};
}  // namespace SD::Nodes

#endif
