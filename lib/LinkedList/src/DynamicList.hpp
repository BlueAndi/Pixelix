/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Dynamic doubly linked list
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef DYNAMIC_LIST_HPP
#define DYNAMIC_LIST_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <functional>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Dynamic doubly linked list.
 * 
 * @tparam T Type of the data which to manage.
 */
template < typename T >
class DynamicList
{
public:

    /**
     * Constructs a dynamic doubly chained empty list.
     * 
     * @param[in] count Max. number of data which shall be handled.
     */
    DynamicList(uint32_t count) :
        m_max(count),
        m_elements(nullptr),
        m_head(nullptr),
        m_tail(nullptr),
        m_count(0U)
    {
        if (0U < count)
        {
            m_elements = new (std::nothrow) ListElement[m_max];

            if (nullptr == m_elements)
            {
                m_max = 0U;
            }
        }
    }

    /**
     * Destroys the dynamic doubly chained list.
     */
    ~DynamicList()
    {
        if (nullptr != m_elements)
        {
            delete[] m_elements;
        }
    }

    /**
     * Copy a list by assignment.
     * 
     * If this list is not empty, you may loose data!
     * If the assigned list is larger, not all elements will be copied.
     *
     * @param[in] list List, which to assign
     * 
     * @return Static double linked list
     */
    DynamicList& operator=(const DynamicList& list)
    {
        if (&list != this)
        {
            ListElement* element = list.m_head;

            clear();
            while(nullptr != element)
            {
                if (false == append(element->data))
                {
                    break;
                }

                element = element->next;
            }
        }

        return *this;
    }

    /**
     * Append data to the list tail.
     * Note, the data will be copied.
     *
     * @param[in] data Element which to append to the tail of the list.
     *
     * @return If data is appended, it will return true otherwise false.
     */
    bool append(T& data)
    {
        bool status = false;

        if (m_max > m_count)
        {
            ListElement* element = getAvailableElement();

            if (nullptr != element)
            {
                /* Empty list? */
                if (nullptr == m_head)
                {
                    element->next   = nullptr;
                    element->prev   = nullptr;

                    m_head          = element;
                    m_tail          = element;
                }
                /* Add to tail. */
                else
                {
                    element->next   = nullptr;
                    element->prev   = m_tail;

                    m_tail->next    = element;
                    m_tail          = element;
                }

                /* Copy element */
                element->data = data;

                ++m_count;

                status = true;
            }
        }

        return status;
    }

    /**
     * Clear list.
     */
    void clear()
    {
        m_head  = nullptr;
        m_tail  = nullptr;
        m_count = 0U;
    }

    /**
     * Get number of elements in the list.
     *
     * @return Number of elements in the list.
     */
    uint32_t getNumOfElements() const
    {
        return m_count;
    }

    /** List element with meta data. */
    struct ListElement
    {
        T               data;   /**< The related data element. */
        ListElement*    next;   /**< Next list element. */
        ListElement*    prev;   /**< Previous list element. */

        /** Ensures that the list element is always initialized. */
        ListElement() :
            data(),
            next(nullptr),
            prev(nullptr)
        {
        }
    };

    /**
     * Basic iterator for the static list which provides
     * general iterator methods.
     */
    template < typename DataType >
    class BasicIterator
    {
    public:

        /** Constructs the basic iterator. */
        BasicIterator() :
            m_current(nullptr),
            m_trash()
        {
        }

        /**
         * Constructs the basic iterator with a selected element.
         * 
         * @param[in] element   The list element, which the iterator shall point to.
         */
        BasicIterator(ListElement* element) :
            m_current(element),
            m_trash()
        {
        }

        /**
         * Constructs the basic iterator by assignment.
         * 
         * @param[in] other Iterator to assign
         */
        BasicIterator(const BasicIterator<DataType>& other) :
            m_current(other.m_current),
            m_trash()
        {
        }

        /**
         * Destroys the basic iterator.
         */
        ~BasicIterator()
        {
        }

        /**
         * Used to check whether iterator points to data.
         * 
         * If iterator is valid, it will return true otherwise false.
         */
        operator bool() const
        {
            bool isValid = false;

            if (nullptr != m_current)
            {
                isValid = true;
            }

            return isValid;
        }

        /**
         * Converts iterator to const iterator only.
         * 
         * @return Const basic iterator
         */
        operator BasicIterator<const DataType>() const
        {
            return BasicIterator<const DataType>(m_current);
        }

        /**
         * Returns a reference to the data.
         * 
         * @return Data reference
         */
        DataType& operator*()
        {
            DataType* data = &m_trash;

            if (nullptr != m_current)
            {
                data = &m_current->data;
            }

            return *data;
        }

        /**
         * Returns a pointer to the data.
         * 
         * @return Data pointer
         */
        DataType* operator->()
        {
            DataType* data = &m_trash;

            if (nullptr != m_current)
            {
                data = &m_current->data;
            }

            return data;
        }

        /**
         * Sets the iterator forwards.
         * 
         * @return Basic iterator reference to next data.
         */
        BasicIterator& operator++()
        {
            if (nullptr != m_current)
            {
                m_current = m_current->next;
            }

            return *this;
        }

        /**
         * Sets the iterator forwards.
         * 
         * @return Basic iterator copy to previous data.
         */
        BasicIterator operator++(int)
        {
            Iterator it = m_current;

            ++(*this);

            return it;
        }

        /**
         * Sets the iterator backwards.
         * 
         * @return Basic iterator reference to previous data.
         */
        BasicIterator& operator--()
        {
            if (nullptr != m_current)
            {
                m_current = m_current->prev;
            }

            return *this;
        }

        /**
         * Sets the iterator backwards.
         * 
         * @return Basic iterator copy to next data.
         */
        BasicIterator operator--(int)
        {
            BasicIterator it = m_current;

            --(*this);

            return it;
        }

        /**
         * Compares the iterators.
         * 
         * @param[in] other Other iterator
         * 
         * @return If iterators pointing to same data, it will return true otherwise false.
         */
        bool operator==(const BasicIterator& other)
        {
            return m_current == other.m_current;
        }

        /**
         * Compares the iterators.
         * 
         * @param[in] other Other iterator
         * 
         * @return If iterators pointing not to the same data, it will return true otherwise false.
         */
        bool operator!=(const BasicIterator& other)
        {
            return m_current != other.m_current;
        }

        /**
         * Assigns iterator.
         * 
         * @param[in] other Ohter iterator which to assign.
         * 
         * @return Basic iterator reference
         */
        BasicIterator& operator=(const BasicIterator& other)
        {
            if (this != &other)
            {
                m_current = other.m_current;
            }

            return *this;
        }

    private:
        ListElement*    m_current;  /** Container element the iterator points to. */
        DataType        m_trash;    /** Trash data, used for illegal iterator access. */

        friend DynamicList;
    };

    /**
     * Static list iterator.
     */
    typedef BasicIterator<T>        Iterator;

    /**
     * Const static list iterator.
     */
    typedef BasicIterator<const T>  ConstIterator;

    /**
     * Returns iterator at begin of list used to iterate forwards.
     * 
     * @return Iterator 
     */
    Iterator begin() const
    {
        return Iterator(m_head);
    }

    /**
     * Returns iterator at end of list, one element behind to determine
     * end of list during forwards iteration.
     * 
     * @return Iterator 
     */
    Iterator end() const
    {
        /* In the end state, the iterator points to the position one past the
         * last element in the list.
         */
        return Iterator(nullptr);
    }

    /**
     * Returns iterator at end of list used to iterate backwards.
     * 
     * @return Iterator 
     */
    Iterator rbegin() const
    {
        return Iterator(m_tail);
    }

    /**
     * Returns iterator at begin of list, one element before to determine
     * end of list during backwards iteration.
     * 
     * @return Iterator 
     */
    Iterator rend() const
    {
        /* In the end state, the iterator points to the position one past the
         * last element in the list.
         */
        return Iterator(nullptr);
    }

    /**
     * Remove element, the iterator points to.
     * 
     * @param[in] it    Iterator which points the list element
     * 
     * @return Iterator of element after the removed one.
     */
    Iterator erase(const Iterator& it)
    {
        ListElement* element = nullptr;

        if (nullptr != it.m_current)
        {
            if (nullptr != it.m_current)
            {
                if (nullptr == it.m_current->next)
                {
                    element = it.m_current->prev;
                }
                else
                {
                    element = it.m_current->next;
                }
            }

            erase(it.m_current);
        }

        return Iterator(element);
    }

    /**
     * Prototype for compare function, used to compare the data user specific.
     */
    typedef std::function<bool(const T& current, const T& toFind)> CompareFunc;

    /**
     * Find the data by compare function.
     * 
     * @param[in] toFind        Data to find
     * @param[in] compareFunc   Compare function
     * 
     * @return If data found, the returned iterator will points to it, otherwise the iterator is invalid.
     */
    Iterator find(const T& toFind, CompareFunc compareFunc) const
    {
        ListElement* element = nullptr;

        if (nullptr != compareFunc)
        {
            element = m_head;
            while(nullptr != element)
            {
                if (true == compareFunc(element->data, toFind))
                {
                    break;
                }

                element = element->next;
            }
        }

        return Iterator(element);
    }

    /**
     * Find the data by compare function.
     * 
     * @param[in] toFind    Data to find
     * 
     * @return If data found, the returned iterator will points to it, otherwise the iterator is invalid.
     */
    Iterator find(const T& toFind) const
    {
        ListElement* element = m_head;

        while(nullptr != element)
        {
            if (0 == memcmp(element->data, toFind, sizeof(T)))
            {
                break;
            }

            element = element->next;
        }

        return Iterator(element);
    }

    /**
     * Prototype for compare function, used to compare the data user specific.
     */
    typedef std::function<bool(const T& current)> FindFunc;

    /**
     * Find the data by compare function.
     * 
     * @param[in] findFunc  Find function which decides whether the right data is found.
     * 
     * @return If data found, the returned iterator will points to it, otherwise the iterator is invalid.
     */
    Iterator find(FindFunc findFunc) const
    {
        ListElement* element = m_head;

        while(nullptr != element)
        {
            if (true == findFunc(element->data))
            {
                break;
            }

            element = element->next;
        }

        return Iterator(element);
    }

private:

    uint32_t        m_max;      /**< Max. number of elements in the list. */
    ListElement*    m_elements; /**< List meta data. */
    ListElement*    m_head;     /**< Head of the list. */
    ListElement*    m_tail;     /**< Tail of the list. */
    uint32_t        m_count;    /**< Number of elements in the list. */

    DynamicList();

    /**
     * Get available list element.
     * 
     * @return If list element is available, it will return its pointer otherwise nullptr.
     */
    ListElement* getAvailableElement()
    {
        ListElement* found = nullptr;

        if (nullptr != m_elements)
        {
            uint32_t idx = 0U;

            while((nullptr == found) && (idx < m_max))
            {
                if ((nullptr == m_elements[idx].next) &&
                    (nullptr == m_elements[idx].prev) &&
                    (m_head != &m_elements[idx]))
                {
                    found = &m_elements[idx];
                }

                ++idx;
            }
        }

        return found;
    }

    /**
     * Remove list element from list.
     *
     * @param[in] element   List element, which to remove.
     */
    void erase(ListElement* element)
    {
        if (nullptr != element)
        {
            /* Is head selected? */
            if (m_head == element)
            {
                /* Last element in the list? */
                if (nullptr == element->next)
                {
                    m_head          = nullptr;
                    m_tail          = nullptr;
                }
                /* Not the last element in the list. */
                else
                {
                    m_head          = element->next;
                    m_head->prev    = nullptr;
                }
            }
            /* Last element in the list? */
            else if (m_tail == element)
            {
                /* Here it is sure, that the list contains more than 1 element. */
                m_tail              = element->prev;
                m_tail->next        = nullptr;
            }
            /* Somewhere between. */
            else
            {
                element->prev->next = element->next;
                element->next->prev = element->prev;
            }

            element->next = nullptr;
            element->prev = nullptr;

            if (0 < m_count)
            {
                --m_count;
            }
        }
    }

    template < typename T0 >
    friend class BasicIterator;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DYNAMIC_LIST_HPP */

/** @} */