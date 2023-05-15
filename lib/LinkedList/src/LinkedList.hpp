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
 * @brief  Doubly linked list
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <new>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * An element in the doubly linked list.
 *
 * @param[in] T Type of element
 */
template < typename T >
class ListElement
{
public:

    /**
     * Constructs an list element.
     *
     * @param[in] element   Element
     * @param[in] prev      Previous list element
     * @param[in] next      Next list element
     */
    ListElement(T element, ListElement* prev, ListElement* next) :
        m_element(element),
        m_prev(prev),
        m_next(next)
    {
    }

    /**
     * Destroy list element.
     */
    ~ListElement()
    {
    }

    /**
     * Get element.
     *
     * @return Element
     */
    T&  getElement()
    {
        return m_element;
    }

    /**
     * Get element.
     *
     * @return Element
     */
    const T&  getElement() const
    {
        return m_element;
    }

    /**
     * Get previous list element.
     *
     * @return Previous list element.
     */
    ListElement* getPrev()
    {
        return m_prev;
    }

    /**
     * Get previous list element.
     *
     * @return Previous list element.
     */
    const ListElement* getPrev() const
    {
        return m_prev;
    }

    /**
     * Set previous list element.
     *
     * @param[in] prev Previous list element
     */
    void setPrev(ListElement* prev)
    {
        m_prev = prev;
    }

    /**
     * Get next list element.
     *
     * @return Next list element.
     */
    ListElement* getNext()
    {
        return m_next;
    }

    /**
     * Get next list element.
     *
     * @return Next list element.
     */
    const ListElement* getNext() const
    {
        return m_next;
    }

    /**
     * Set next list element.
     *
     * @param[in] next Next list element
     */
    void setNext(ListElement* next)
    {
        m_next = next;
    }

private:

    T               m_element;  /**< Element */
    ListElement*    m_prev;     /**< Previous list element */
    ListElement*    m_next;     /**< Next list element */

    ListElement();
    ListElement(const ListElement& listElement);
    ListElement& operator=(const ListElement& listElement);

};

template < typename T >
class DLinkedList;

/**
 * Doubly linked list iterator.
 */
template < typename T >
class DLinkedListIterator
{
public:

    /**
     * Constructs a iterator for the doubly linked list.
     *
     * @param[in] list  Doubly linked list
     */
    DLinkedListIterator(DLinkedList<T>& list) :
        m_list(list),
        m_curr(list.m_head)
    {
    }

    /**
     * Destroys the iterator of the doubly linked list.
     */
    ~DLinkedListIterator()
    {
    }

    /**
     * Select first element.
     *
     * @return If list is empty, it will return false otherwise true.
     */
    bool first()
    {
        bool status = false;

        if (nullptr != m_list.m_head)
        {
            m_curr = m_list.m_head;
            status = true;
        }

        return status;
    }

    /**
     * Select last element.
     *
     * @return If list is empty, it will return false otherwise true.
     */
    bool last()
    {
        bool status = false;

        if (nullptr != m_list.m_tail)
        {
            m_curr = m_list.m_tail;
            status = true;
        }

        return status;
    }

    /**
     * Select next element in the list.
     * If the current selected element is the last element, it will return false
     * otherwise true.
     *
     * @return If the current selected element is the last element, it will return false otherwise true.
     */
    bool next()
    {
        bool status = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            if (nullptr != m_curr->getNext())
            {
                m_curr = m_curr->getNext();
                status = true;
            }
        }

        return status;
    }

    /**
     * Select previous element in the list.
     * If the current selected element is the first element, it will return false
     * otherwise true.
     *
     * @return If the current selected element is the first element, it will return false otherwise true.
     */
    bool prev()
    {
        bool status = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            if (nullptr != m_curr->getPrev())
            {
                m_curr = m_curr->getPrev();
                status = true;
            }
        }

        return status;
    }

    /**
     * Get current selected element.
     *
     * @return Selected element
     */
    T* current()
    {
        T* elem = nullptr;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            elem = &m_curr->getElement();
        }

        return elem;
    }

    /**
     * Search for a specific element in the list and select it.
     * It starts searching from the current selected element till end of the list.
     * If element is not found, the last element in the list is selected.
     *
     * @param[in] element   Element to find
     */
    bool find(const T& element)
    {
        bool found = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            do
            {
                if (element == m_curr->getElement())
                {
                    found = true;
                }
            }
            while((false == found) && (true == next()));
        }

        return found;
    }

    /**
     * Remove selected element from list.
     */
    void remove()
    {
        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        m_list.remove(m_curr);
        m_curr = m_list.m_head;
    }

private:

    DLinkedList<T>& m_list;     /**< Doubly linked list */
    ListElement<T>* m_curr;     /**< Current selected list element */

    DLinkedListIterator();
};

/**
 * Doubly linked list const iterator.
 */
template < typename T >
class DLinkedListConstIterator
{
public:

    /**
     * Constructs a const iterator for the doubly linked list.
     *
     * @param[in] list  Doubly linked list
     */
    DLinkedListConstIterator(const DLinkedList<T>& list) :
        m_list(list),
        m_curr(list.m_head)
    {
    }

    /**
     * Destroys the iterator of the doubly linked list.
     */
    ~DLinkedListConstIterator()
    {
    }

    /**
     * Select first element.
     *
     * @return If list is empty, it will return false otherwise true.
     */
    bool first()
    {
        bool status = false;

        if (nullptr != m_list.m_head)
        {
            m_curr = m_list.m_head;
            status = true;
        }

        return status;
    }

    /**
     * Select last element.
     *
     * @return If list is empty, it will return false otherwise true.
     */
    bool last()
    {
        bool status = false;

        if (nullptr != m_list.m_tail)
        {
            m_curr = m_list.m_tail;
            status = true;
        }

        return status;
    }

    /**
     * Select next element in the list.
     * If the current selected element is the last element, it will return false
     * otherwise true.
     *
     * @return If the current selected element is the last element, it will return false otherwise true.
     */
    bool next()
    {
        bool status = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            if (nullptr != m_curr->getNext())
            {
                m_curr = m_curr->getNext();
                status = true;
            }
        }

        return status;
    }

    /**
     * Select previous element in the list.
     * If the current selected element is the first element, it will return false
     * otherwise true.
     *
     * @return If the current selected element is the first element, it will return false otherwise true.
     */
    bool prev()
    {
        bool status = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            if (nullptr != m_curr->getPrev())
            {
                m_curr = m_curr->getPrev();
                status = true;
            }
        }

        return status;
    }

    /**
     * Get current selected element.
     *
     * @return Selected element
     */
    const T* current()
    {
        const T* elem = nullptr;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            elem = &m_curr->getElement();
        }

        return elem;
    }

    /**
     * Search for a specific element in the list and select it.
     * It starts searching from the current selected element till end of the list.
     * If element is not found, the last element in the list is selected.
     *
     * @param[in] element   Element to find
     */
    bool find(const T& element)
    {
        bool found = false;

        /* In case that the list was empty at the time the
         * iterator was created, the current selected element
         * is nullptr.
         */
        if (nullptr == m_curr)
        {
            m_curr = m_list.m_head;
        }

        if (nullptr != m_curr)
        {
            do
            {
                if (element == m_curr->getElement())
                {
                    found = true;
                }
            }
            while((false == found) && (true == next()));
        }

        return found;
    }

private:

    const DLinkedList<T>&   m_list; /**< Doubly linked list */
    const ListElement<T>*   m_curr; /**< Current selected list element */

    DLinkedListConstIterator();
};

/**
 * Doubly linked list.
 *
 * @param[in] T Type of element
 */
template < typename T >
class DLinkedList
{
public:

    /**
     * Constructs a double chained empty list.
     */
    DLinkedList() :
        m_head(nullptr),
        m_tail(nullptr),
        m_count(0U)
    {
    }

    /**
     * Constructs a double chained list, by copying an existing one.
     *
     * @param[in] list List, which to copy
     */
    DLinkedList(const DLinkedList& list) :
        m_head(nullptr),
        m_tail(nullptr),
        m_count(0U)
    {
        ListElement<T>* listElement = list.m_head;

        while(nullptr != listElement)
        {
            append(listElement->getElement());
            listElement = listElement->getNext();
        }
    }

    /**
     * Destroys the double chained list.
     */
    ~DLinkedList()
    {
        clear();
    }

    /**
     * Assign a list, including its elements.
     * Attention, if this list is not empty, you may loose data!
     *
     * @param[in] list List, which to assign
     */
    DLinkedList& operator=(const DLinkedList& list)
    {
        if (&list != this)
        {
            ListElement<T>* listElement = list.m_head;

            clear();
            while(nullptr != listElement)
            {
                append(listElement->getElement());
                listElement = listElement->getNext();
            }
        }

        return *this;
    }

    /**
     * Append element to the list tail.
     *
     * @param[in] element New element in the list.
     *
     * @return If element is appended, it will return true otherwise false.
     */
    bool append(T& element)
    {
        bool status = false;

        if (UINT32_MAX > m_count)
        {
            ListElement<T>* listElement = new(std::nothrow) ListElement<T>(element, m_tail, nullptr);

            if (nullptr != listElement)
            {
                /* Empty list? */
                if (nullptr == m_head)
                {
                    m_head = listElement;
                    m_tail = listElement;
                }
                else
                {
                    m_tail->setNext(listElement);
                    m_tail = listElement;
                }

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
        ListElement<T>* curr = m_head;

        while(nullptr != curr)
        {
            /* Last element in the list? */
            if (nullptr == curr->getNext())
            {
                delete curr;
                curr = nullptr;
            }
            else
            {
                curr = curr->getNext();
                delete curr->getPrev();
                curr->setPrev(nullptr);
            }
        }

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

private:

    ListElement<T>* m_head;     /**< Head of list */
    ListElement<T>* m_tail;     /**< Tail of list */
    uint32_t        m_count;    /**< Number of elements in the list */

    /**
     * Remove element from list.
     *
     * @param[in] listElement   List element, which to remove
     */
    void remove(ListElement<T>* listElement)
    {
        if (nullptr != listElement)
        {
            /* Is head selected? */
            if (m_head == listElement)
            {
                /* Last element in the list */
                if (nullptr == listElement->getNext())
                {
                    delete listElement;
                    m_head = nullptr;
                    m_tail = nullptr;
                    listElement = nullptr;
                }
                /* Not the last element in the list */
                else
                {
                    m_head = listElement->getNext();
                    m_head->setPrev(nullptr);
                    delete listElement;
                    listElement = m_head;
                }
            }
            /* Last element in the list? */
            else if (m_tail == listElement)
            {
                /* Here it is sure, that the list contains more than 1 element. */
                m_tail = listElement->getPrev();
                m_tail->setNext(nullptr);
                delete listElement;
                listElement = m_tail;
            }
            /* Somewhere between */
            else
            {
                listElement->getPrev()->setNext(listElement->getNext());
                listElement->getNext()->setPrev(listElement->getPrev());
                delete listElement;
            }

            if (0 < m_count)
            {
                --m_count;
            }
        }
    }

    template < typename T0 >
    friend class DLinkedListIterator;

    template < typename T1 >
    friend class DLinkedListConstIterator;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LINKEDLIST_HPP */

/** @} */