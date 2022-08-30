// xpsRBTree.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsRBTree_h_
#define _xpsRBTree_h_

#include "xpsSal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_RBT_NODE_CLEAR_ALL     0x00
#define XP_RBT_NODE_VISIT         0x1
#define XP_RBT_NODE_ADDR_CONVERT  0x2

typedef enum xpsColor_e
{
    RED=0,
    BLACK,
    RED_BLACK,
    BLACK_BLACK
} xpsColor_e;

/**
 * \typedef (*xpsComp_t)
 * \brief Function pointer type for key comparison
 *
 * This function pointer allow for the user to specify how to
 * compare two keys of any arbitrary type. The compare function
 * must satisfy the following rules:
 *
 * - Key 1 < Key 2 returns -1
 * - Key 1 == Key 2 returns 0
 * - Key 1 > Key 2 returns 1
 */
typedef int32_t (*xpsComp_t)(void* key1, void* key2);

typedef struct xpsRBNode_t
{
    struct xpsRBNode_t* llink;
    struct xpsRBNode_t* rlink;
    struct xpsRBNode_t* parent;
    void *data;
    xpsColor_e color;
    uint8_t isVisitAndConvert;
} xpsRBNode_t;

typedef struct xpsRBTree_t
{
    xpsRBNode_t *root;
    /* Compare function set during tree init */
    xpsComp_t comp;
    /* Version details */
    uint32_t version;
    /* Number of nodes in the tree. */
    uint32_t count;
    /* lock can be added here */
} xpsRBTree_t;

/**
 * \brief Creates the tree and handles initialization.
 *
 *
 * \param [in] treePtr  returns pointer to the tree created.
 * \param [in] comp  compare function passed by app.
 * \param [in] version  Used for potablity between versions.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsRBTInit(xpsRBTree_t** treePtr, xpsComp_t comp, int version);

/**
 * \brief Deletes the tree and all of its nodes.
 *
 *
 * \param [in] treePtr
 *
 * \return
 */
XP_STATUS xpsRBTDelete(xpsRBTree_t* treePtr);

/**
 * \brief Cleans up the tree(release memory of the nodes of the tree).
 *
 * \param [in] treePtr
 *
 * \return
 */
XP_STATUS xpsRBTPurge(xpsRBTree_t* treePtr);

/**
 * \brief Returns the number of nodes in the tree
 *
 * \param [in] treePtr
 *
 * \return
 */
int xpsRBTCountNodes(xpsRBTree_t* treePtr);

/**
 * \brief Add node to tree
 *
 *
 * \param [in] treePtr
 * \param [in] data - pointer to data.
 *
 * \return XP_ERR_KEY_EXISTS if key exists or XP_NO_ERR
 *         on inserting a node.
 */
XP_STATUS xpsRBTAddNode(xpsRBTree_t* treePtr, void* data);

/**
 * \brief Returns NULL if node is not found
 *        else pointer to data.
 *
 * \param [in] treePtr
 * \param [in] key
 *
 * \return void*
 */
void* xpsRBTSearchNode(xpsRBTree_t* treePtr, void* key);

/**
 * \brief Deletes node from tree.Memory is freed by application
 *        Returns NULL if node is not found
 *        else pointer to data.
 *
 * \param [in] treePtr pointer to data.
 * \param [in] key
 *
 * \return void*
 */
void* xpsRBTDeleteNode(xpsRBTree_t* treePtr, void* key);

/**
 * \brief If key is NULL, first node of inorder traversal
 *        is returned else pointer to data.
 *        If key is not present in tree, NULL is returned.
 *
 * \param [in] treePtr
 * \param [in] key - pointer to data
 *
 * \return void*
 */
void* xpsRBTGetInorderSuccessor(xpsRBTree_t* treePtr, void* key);

/**
 * \brief Update memory address of all nodes in the tree
 *
 *
 * \param [in] * treePtr
 *
 * \return XP_NO_ERR on Sucess.
 */
XP_STATUS xpsRBTUpdateMemoryAddress(xpsRBTree_t* treePtr);

#ifdef __cplusplus
}
#endif
#endif  //_xpsRBTree_h_
