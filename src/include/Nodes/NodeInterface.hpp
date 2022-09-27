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

#ifndef __SD__NODES__NODE_INTERFACE__
#define __SD__NODES__NODE_INTERFACE__

namespace SD::Nodes {
    /**
     * @class   NodeInterface
     * @brief   The interface which all nodes must adhere
     *
     * Since templates cannot be implemented in a .cpp file, we are using the
     * undefined functions as a way to ensure that this class can't be
     * instantiated. This gives us the same functionality as an abstract class
     * (interface) without forcing us to give up templates.
     */
    template<typename T>
    class NodeInterface
    {
        /**
         * @brief   set the node's value
         * @param   value   value to set
         * @return  *self
         */
        NodeInterface<T>& setValue(T value);

        /**
         * @brief   retrieve the node's value
         * @return  contained value
         */
        T getValue();
    };
}

#endif
