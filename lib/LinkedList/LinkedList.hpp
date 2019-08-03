/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Doubly linked list
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides a double chained list.

*******************************************************************************/
/** @defgroup linkedlist Doubly linked list
 * This list is a doubly linked list.
 *
 * @{
 */

#ifndef __LINKEDLIST_HPP__
#define __LINKEDLIST_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

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
     * Get element.
     * 
     * @return Element
     */
    T&  getElement(void)
    {
        return m_element;
    }

    /**
     * Get previous list element.
     * 
     * @return Previous list element.
     */
    ListElement* getPrev(void)
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
        return;
    }

    /**
     * Get next list element.
     * 
     * @return Next list element.
     */
    ListElement* getNext(void)
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
        return;
    }

private:

    T               m_element;  /**< Element */
    ListElement*    m_prev;     /**< Previous list element */
    ListElement*    m_next;     /**< Next list element */

    ListElement();
    ListElement(const ListElement& listElement);
    ListElement& operator=(const ListElement& listElement);

};

/**
 * Doubly linked list.
 * 
 * @param[in] T Type of element
 */
template < typename T >
class LinkedList
{
public:

    /**
     * Constructs a double chained empty list.
     */
    LinkedList() :
        m_head(NULL),
        m_tail(NULL),
        m_curr(NULL)
    {
    }

    /**
     * Constructs a double chained list, by copying an existing one.
     * 
     * @param[in] list List, which to copy
     */
    LinkedList(const LinkedList& list) :
        m_head(NULL),
        m_tail(NULL),
        m_curr(NULL)
    {
        ListElement<T>* listElement = list.m_head;

        while(NULL != listElement)
        {
            append(listElement->getElement());
            listElement = listElement->getNext();
        }
    }

    /**
     * Destroys the double chained list.
     */
    ~LinkedList()
    {
        clear();
    }

    /**
     * Assign a list, including its elements.
     * Attention, if this list is not empty, you may loose data!
     * 
     * @param[in] list List, which to assign
     */
    LinkedList& operator=(const LinkedList& list)
    {
        ListElement<T>* listElement = list.m_head;

        clear();
        while(NULL != listElement)
        {
            append(listElement->getElement());
            listElement = listElement->getNext();
        }

        return;
    }

    /**
     * Get first element in the list.
     * If the list is empty, NULL will be returned.
     * 
     * @return First element.
     */
    T* first(void) const
    {
        T*  element = NULL;

        if (NULL != m_head)
        {
            element = &m_head->getElement();
        }

        return element;
    }

    /**
     * Get last element in the list.
     * If the list is empty, NULL will be returned.
     * 
     * @return Last element.
     */
    T* last(void) const
    {
        T*  element = NULL;

        if (NULL != m_tail)
        {
            element = &m_tail->getElement();
        }

        return element;
    }

    /**
     * Get current selected element in the list.
     * If the list is empty, NULL will be returned.
     * 
     * @return Current element.
     */
    T* current(void) const
    {
        T*  element = NULL;

        if (NULL != m_curr)
        {
            element = &m_curr->getElement();
        }

        return element;
    }

    /**
     * Append element to the list tail.
     * 
     * @param[in] element New element in the list.
     */
    void append(T element)
    {
        ListElement<T>* listElement = new ListElement<T>(element, m_tail, NULL);

        if (NULL != listElement)
        {
            /* Empty list? */
            if (NULL == m_head)
            {
                m_head = listElement;
                m_tail = listElement;
                m_curr = listElement;
            }
            else
            {
                m_tail->setNext(listElement);
                m_tail = listElement;
            }
        }

        return;
    }

    /**
     * Select next element in the list.
     * If the current selected element is the last element, it will return false
     * otherwise true.
     * 
     * @return If the current selected element is the last element, it will return false otherwise true.
     */
    bool next(void)
    {
        bool status = false;

        if (NULL != m_curr)
        {
            if (NULL != m_curr->getNext())
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
    bool prev(void)
    {
        bool status = false;

        if (NULL != m_curr)
        {
            if (NULL != m_curr->getPrev())
            {
                m_curr = m_curr->getPrev();
                status = true;
            }
        }

        return status;
    }

    /**
     * Select first element.
     */
    void selectFirstElement(void)
    {
        if (NULL != m_head)
        {
            m_curr = m_head;
        }

        return;
    }

    /**
     * Select last element.
     */
    void selectLastElement(void)
    {
        if (NULL != m_head)
        {
            m_curr = m_tail;
        }

        return;
    }

    /**
     * Remove selected element from list.
     */
    void removeSelected(void)
    {
        if (NULL != m_curr)
        {
            /* Is head selected? */
            if (m_head == m_curr)
            {
                /* Last element in the list */
                if (NULL == m_curr->getNext())
                {
                    delete m_curr;
                    m_head = NULL;
                    m_tail = NULL;
                    m_curr = NULL;
                }
                /* Not the last element in the list */
                else
                {
                    m_head = m_curr->getNext();
                    m_head->setPrev(NULL);
                    delete m_curr;
                    m_curr = m_head;
                }
            }
            /* Last element in the list? */
            else if (m_tail == m_curr)
            {
                /* Here it is sure, that the list contains more than 1 element. */
                m_tail = m_curr->getPrev();
                m_tail->setNext(NULL);
                delete m_curr;
                m_curr = m_tail;
            }
            /* Somewhere between */
            else
            {
                ListElement<T>* tmp = m_curr->getNext();

                m_curr->getPrev()->setNext(m_curr->getNext());
                m_curr->getNext()->setPrev(m_curr->getPrev());
                delete m_curr;
                m_curr = tmp;
            }
        }
        return;
    }

    /**
     * Clear list.
     */
    void clear(void)
    {
        m_curr = m_head;

        while(NULL != m_curr)
        {
            /* Last element in the list? */
            if (NULL == m_curr->getNext())
            {
                delete m_curr;
                m_curr = NULL;
            }
            else
            {
                m_curr = m_curr->getNext();
                delete m_curr->getPrev();
                m_curr->setPrev(NULL);
            }
        }

        m_head = NULL;
        m_tail = NULL;

        return;
    }

private:

    ListElement<T>* m_head; /**< Head of list */
    ListElement<T>* m_tail; /**< Tail of list */
    ListElement<T>* m_curr; /**< Current selected list element */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __LINKEDLIST_HPP__ */

/** @} */