/* list.h
 *
 * The List API for manipulation of a list of elements.
 *
 * This API can be used to implement lists of variable size.
 * The list elements must be allocated statically or dynamically by the API
 * user.
 *
 * The list instances can be allocated statically or dynamically.
 *
 * Note: the support for static list instances will be deprecated
 *       in future versions. Do not use it anymore!
 */

/*****************************************************************************
* Copyright (c) 2012-2015 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

#ifndef LIST_H_
#define LIST_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Dummy list ID to use with the List API */
#define LIST_DUMMY_LIST_ID          0

#define LIST_INTERNAL_DATA_SIZE     2

/*----------------------------------------------------------------------------
 * List_Status_t
 *
 * Return values for all the API functions.
 */
typedef enum
{
    LIST_STATUS_OK,
    LIST_ERROR_BAD_ARGUMENT,
    LIST_ERROR_INTERNAL
} List_Status_t;


/*----------------------------------------------------------------------------
 * List_Element_t
 *
 * The element data structure.
 *
 */
typedef struct
{
    /* Pointer to a data object associated with this element, */
    /* can be filled in by the API user */
    void * DataObject_p;

    /* Data used internally by the API implementation only, */
    /* may not be written by the API user */
    void * Internal[LIST_INTERNAL_DATA_SIZE];

} List_Element_t;

typedef struct
{
    /* List head */
    List_Element_t * Head_p;

    /* List tail */
    List_Element_t * Tail_p;

    /* Number of elements in the list */
    unsigned int ElementCount;

} List_t;

/*----------------------------------------------------------------------------
 * List_Init
 *
 * Initializes a list instance. The list is empty when this function
 * returns. The List_Add() function must be used to populate the list
 * instance with the elements.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * This function must be called before any other function
 * for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Handle_p was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_Init(
        const unsigned int ListID,
        void * const ListInstance_p);


/*----------------------------------------------------------------------------
 * List_Uninit
 *
 * Uninitializes the requested list instance. All the resources
 * associated with this instance will be freed before this function returns.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * This function must be called in the end when the list instance is not needed
 * anymore.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, instance is uninitialized.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_Uninit(
        const unsigned int ListID,
        void * const ListInstance_p);


/*----------------------------------------------------------------------------
 * List_AddToHead
 *
 * Adds an element to the list head. The element will be added to
 * the list instance when this function returns LIST_STATUS_OK.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Element_p (input)
 *     Pointer to the element to be added to the list instance. This element
 *     may not be already present in the list. Cannot be NULL.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_AddToHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t * const Element_p);


/*----------------------------------------------------------------------------
 * List_RemoveFromTail
 *
 * Removes an element from the list tail. The element will be removed from
 * the list instance when this function returns LIST_STATUS_OK.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Element_pp (output)
 *     Pointer to the memory location where the element from the list
 *     instance will be stored.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Element_pp was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_RemoveFromTail(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t ** const Element_pp);


/*----------------------------------------------------------------------------
 * List_RemoveAnywhere
 *
 * Removes requested element from the list. The element will be removed from
 * the list instance when this function returns LIST_STATUS_OK.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Element_p (input/output)
 *     Pointer to the memory location where the element to be removed from
 *     the list instance is stored.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Element_pp was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_RemoveAnywhere(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t * const Element_p);


/*----------------------------------------------------------------------------
 * List_GetListElementCount
 *
 * Gets the number of elements added to the list.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Count_p (output)
 *     Pointer to the memory location where the list element count
 *     will be stored.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Count_p was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_GetListElementCount(
        const unsigned int ListID,
        void * const ListInstance_p,
        unsigned int * const Count_p);


/*----------------------------------------------------------------------------
 * List_GetHead
 *
 * Gets the list head.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Element_pp (output)
 *     Pointer to the memory location where the element from the list
 *     instance will be stored.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Handle_p was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_GetHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        const List_Element_t ** const Element_pp);


/*----------------------------------------------------------------------------
 * List_RemoveFromHead
 *
 * Removes an element from the list head. The element will be removed from
 * the list instance when this function returns LIST_STATUS_OK.
 *
 * ListID (input)
 *     List instance identifier to be initialized.
 *     Ignored if ListInstance_p is not NULL.
 *
 * ListInstance_p (input)
 *     Pointer to the List instance to be initialized.
 *     If the ListID is used then ListInstance_p can be set to NULL.
 *
 * Element_pp (output)
 *     Pointer to the memory location where the element from the list
 *     instance will be stored.
 *
 * This function is re-entrant for different list instances.
 * This function is not re-entrant for the same list instance.
 *
 * Return Values
 *     LIST_STATUS_OK: Success, Handle_p was written.
 *     LIST_ERROR_BAD_ARGUMENT: Invalid input parameter.
 */
List_Status_t
List_RemoveFromHead(
        const unsigned int ListID,
        void * const ListInstance_p,
        List_Element_t ** const Element_pp);


/*----------------------------------------------------------------------------
 * List_GetNextElement
 *
 * Gets the next element for the provided one.
 *
 * Element_p (input)
 *     Pointer to the element for which the next element must be obtained.
 *     Cannot be NULL.
 *
 * This function is re-entrant.
 *
 * Return Values
 *     Pointer to the next element or NULL if not found.
 */
List_Element_t *
List_GetNextElement(const List_Element_t * const Element_p);



/*----------------------------------------------------------------------------
 * List_GetInstanceByteCount
 *
 * Gets the memory size of the list instance (in bytes) excluding the list
 * elements memory size. This list memory size can be used to allocate a list
 * instance and pass a pointer to it subsequently to the List_*() functions.
 *
 * This function is re-entrant and can be called any time.
 *
 * Return Values
 *     Size of the list administration memory in bytes.
 */
unsigned int
List_GetInstanceByteCount(void);


#endif /* LIST_H_ */


/* end of file list.h */
