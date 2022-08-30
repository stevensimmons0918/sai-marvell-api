// xpsRBTree.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsRBTree.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpsRBTree.c
 * \brief Contains the implementation of the public and private
 *        Rb Tree API
 *
 * This file contains the xps implementation of the Red-Black
 * Tree. A Red-Black tree is a self-balancing binary search tree
 * which allows for fast binary search given a unique key. We
 * chose a Red-Black tree over other impelmentations of trees
 * for the following reasons:
 *
 * - A pure Binary Search Tree (BST) can be easily skewed either
 *   right or left leading to a bad search time
 *
 * - AVL's have a much slower rotational time, even though it
 *   can be on average a lower height
 *
 * - Our use case has quite a fair amount of random key/data
 *   insertion, where we prefer a faster insert/deletion time
 *   over the incremental search time improvements
 *
 * In addition a Red-Black tree is a very well known and common
 * solution to fast self-balancing binary trees. This will be
 * used as a fundamental database structure inside XPS
 */

/*
 * Enums private to this file
 */
typedef enum xpsDelCase_e
{
    CASE1L=0,
    CASE1R,
    CASE2,
    CASE3L,
    CASE3R,
    CASE4L,
    CASE4R,
    CASE5,
    CASE_INVALID
} xpsDelCase_e;

/*
 * Static function prototypes
 */
static void         xpsRBTSetParentDel(xpsRBTree_t *treePtr, xpsRBNode_t *node,
                                       xpsRBNode_t *child);
static void         xpsRBTFreeNode(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTGetNewNode(xpsRBTree_t *treePtr, void *data);
static xpsRBNode_t* xpsRBTGetGrandParent(xpsRBNode_t* node);
static xpsRBNode_t* xpsRBTGetUncle(xpsRBNode_t* node);
static void         xpsRBTRotateLeft(xpsRBTree_t* treePtr, xpsRBNode_t* node);
static void         xpsRBTColorRotate(xpsRBTree_t* treePtr, xpsRBNode_t* node);
static void         xpsRBTRotate(xpsRBTree_t* treePtr, xpsRBNode_t* node);
static void         xpsRBTFixRedBlackTree(xpsRBTree_t* treePtr,
                                          xpsRBNode_t* node);
static xpsRBNode_t* xpsRBTGetMin(xpsRBTree_t *treePtr, xpsRBNode_t *root,
                                 xpsRBNode_t **delSuccesor, xpsRBNode_t* dummyNode);
static xpsRBNode_t* xpsRBTGetSibling(xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTDelCase1L(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTDelCase1R(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static void         xpsRBTRemoveExtraBNode(xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTDelCase2(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTDelCase3L(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static xpsRBNode_t* xpsRBTDelCase3R(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static void         xpsRBTDelCase4L(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static void         xpsRBTDelCase4R(xpsRBTree_t *treePtr, xpsRBNode_t *node);
static uint8_t      xpsRBTIsBlack(xpsRBNode_t *node);
static xpsDelCase_e xpsRBTGetDelCase(xpsRBNode_t *node);
static void         xpsRBTDelFixRBTree(xpsRBTree_t *treePtr, xpsRBNode_t *n);
static xpsRBNode_t* xpsRBTGetMinPtr(xpsRBNode_t *root);
static xpsRBNode_t* xpsRBTSearchNodePtr(xpsRBNode_t *root, xpsComp_t comp,
                                        void* key);
static xpsRBNode_t* xpsGetNextInorderNode(xpsRBTree_t* treePtr,
                                          xpsRBNode_t *node);

/**
 * \fn xpsRBTInit
 * \brief Creates the tree and handles initialization.
 *
 *
 * \param [in] treePtr - returns pointer to the tree created.
 * \param [in] comp - compare function passed by app.
 * \param [in] version - Used for potablity between versions.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsRBTInit(xpsRBTree_t** treePtr, xpsComp_t comp, int version)
{
    xpsRBTree_t* tPtr=NULL;

    if ((comp==NULL) || (treePtr == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Allocate memory and initialize tree. */
    tPtr = (xpsRBTree_t*)XP_PERSISTENT_MALLOC(sizeof(xpsRBTree_t));
    if (!tPtr)
    {
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(tPtr, 0, sizeof(xpsRBTree_t));
    tPtr->comp = comp;
    tPtr->version = version;
    tPtr->root = NULL;

    *treePtr = tPtr;
    return XP_NO_ERR;
}

XP_STATUS xpsRBTDelete(xpsRBTree_t* treePtr)
{
    if (treePtr == NULL)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    xpsRBTPurge(treePtr);
    XP_PERSISTENT_FREE(treePtr);
    treePtr = NULL;

    return XP_NO_ERR;
}

XP_STATUS xpsRBTPurge(xpsRBTree_t* treePtr)
{
    xpsRBNode_t *root = NULL, *temp = NULL;

    if (treePtr == NULL)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    root = treePtr->root;

    while (1)
    {
        if (root == NULL)
        {
            return XP_NO_ERR;
        }

        while ((root != NULL) && (root->llink != NULL))
        {
            root = root->llink;
        }
        /* Go to the left most node of the right subtree. */
        if (root->rlink != NULL)
        {
            root = xpsRBTGetMinPtr(root->rlink);
            /* We need to go through all cases as
             * minNode's right child can have two children. */
            continue;
        }

        /* If node exist to the right side of minNode. go t */
        while ((root != NULL) && (root->rlink != NULL))
        {
            root = root->rlink;
        }
        /* root is pointing to a leaf. Store parent address
         * and free the node. */
        if (root != NULL)
        {
            temp = root;
            root = root->parent;
        }

        /* Set the  parent link to null. */
        if (root == NULL)
        {
            /* Root node is also deleted. Update the root pointer in tree. */
            treePtr->root = NULL;
        }
        else
        {
            if (root->llink == temp)
            {
                root->llink = NULL;
            }
            else
            {
                root->rlink = NULL;
            }
        }

        if (temp != NULL)
        {
            /* Free the memory of the data also
             * as we can't return pointer to app */
            if (temp->data != NULL)
            {
                XP_PERSISTENT_FREE(temp->data);
            }

            xpsRBTFreeNode(treePtr, temp);
        }
    }

    return XP_NO_ERR;
}

int xpsRBTCountNodes(xpsRBTree_t* treePtr)
{
    return treePtr->count;
}

XP_STATUS xpsRBTAddNode(xpsRBTree_t* treePtr, void* data)
{
    xpsRBNode_t* prev=NULL, *root=NULL, *node=NULL;
    xpsComp_t comp;

    if (treePtr == NULL)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    comp = treePtr->comp;
    root = treePtr->root;

    if ((node = xpsRBTGetNewNode(treePtr, data)) == NULL)
    {
        return XP_ERR_OUT_OF_MEM;
    }

    if (root == NULL)
    {
        node->color = BLACK;
        treePtr->root = node;
        return XP_NO_ERR;
    }

    while (root != NULL)
    {
        if ((*comp)(node->data, root->data) < 0)
        {
            prev = root;
            root = root->llink;
        }
        else if ((*comp)(node->data, root->data) > 0)
        {
            prev = root;
            root = root->rlink;
        }
        else
        {
            /* Free the allocated memory as node not inserted. */
            xpsRBTFreeNode(treePtr, node);
            return XP_ERR_KEY_EXISTS;
        }
    }

    if ((*comp)(node->data, prev->data) < 0)
    {
        prev->llink = node;
    }
    else
    {
        prev->rlink = node;
    }
    node->parent = prev;

    xpsRBTFixRedBlackTree(treePtr, node);
    return XP_NO_ERR;
}

void* xpsRBTSearchNode(xpsRBTree_t* treePtr, void* key)
{
    xpsRBNode_t *node=NULL;

    if (treePtr == NULL)
    {
        return NULL;
    }

    node = xpsRBTSearchNodePtr(treePtr->root, treePtr->comp, key);

    if (node != NULL)
    {
        return node->data;
    }

    return NULL;
}

void* xpsRBTDeleteNode(xpsRBTree_t* treePtr, void* key)
{
    xpsRBNode_t *root = NULL;
    xpsComp_t comp;
    void *data=NULL;

    if (treePtr == NULL)
    {
        return NULL;
    }

    root = treePtr->root;
    comp = treePtr->comp;

    if (root == NULL)
    {
        return NULL;
    }

    while (root != NULL)
    {
        if ((*comp)(key, root->data) < 0)
        {
            root=root->llink;
        }
        else if ((*comp)(key, root->data) > 0)
        {
            root=root->rlink;
        }
        else
        {
            break;
        }
    }

    if (root == NULL)
    {
        return NULL;
    }
    /* root- points to the node to be deleted  */

    /* The node which is getting deleted can be replaced
     * smallest in left subtree or largest in right subtree.
     * minNode is the smallest in right subtree. It takes the
     * color of root node. If minNode was black, it affects the
     * black height of tree. delSuccesor(minNode child) gets this
     * extra black. It's fixed in xpsRBTDelFixRBTree.
     */
    xpsRBNode_t *minNode = NULL;
    xpsRBNode_t *delSuccesor = NULL;
    xpsColor_e clr;
    xpsRBNode_t dummyNode= {0};

    if (root->llink == NULL)
    {
        /* no children */
        if (root->rlink == NULL)
        {
            /* Deleting a black node(root) creates black height imbalance.
             * Add a dummy node for right child(leaf) of root.
             * Color it BLACK_BLACK as it is leaf. */
            if (root->parent == NULL)
            {
                /* This node is rootnode. No need of color fix. */
                xpsRBTSetParentDel(treePtr, root, root->rlink);
                goto CLEANUP;
            }

            if (root->color == BLACK)
            {
                root->rlink = &dummyNode;
                dummyNode.color = BLACK_BLACK;
            }

            xpsRBTSetParentDel(treePtr, root, root->rlink);

            /* Fix color of BLACK_BLACK. */
            delSuccesor = &dummyNode;
            xpsRBTDelFixRBTree(treePtr, delSuccesor);
            goto CLEANUP;
        }
        /* only right child, no children case. */
        root->rlink->color = BLACK;

        xpsRBTSetParentDel(treePtr, root, root->rlink);
    }
    else if (root->rlink == NULL)
    {
        /* only left child case */
        root->llink->color = BLACK;

        xpsRBTSetParentDel(treePtr, root, root->llink);
    }
    else
    {
        /* Two children case. Right tree minimum is put in place
         * of root(left tree maximum can also be used).
         * Adjust the pointer accordingly. */

        minNode = xpsRBTGetMin(treePtr, root->rlink, &delSuccesor, &dummyNode);

        /* Fix root<->rootchilden conn to minnode-<>rootchilden conn. */
        minNode->llink = root->llink;
        root->llink->parent = minNode;

        /*
         * if minnode is blk, right child exist(dummy or usual)
         * if clr is RED, right child was null. no action reqd.
         *  */
        minNode->rlink = root->rlink;
        if (root->rlink!=NULL)
        {
            root->rlink->parent = minNode;
        }

        /* Fixes link between root--rootParent connection to minNode-<>root parent*/
        xpsRBTSetParentDel(treePtr, root, minNode);

        /* color the successor(minNode) with root color.
         * If successor was black, add this extra black
         * to child which replaces the succesor. Then fix it. */
        clr = minNode->color;
        minNode->color = root->color;

        /* delSuccesor can be null. */
        if (clr == BLACK)
        {
            xpsRBTDelFixRBTree(treePtr, delSuccesor);
        }
    }
CLEANUP:
    data = root->data;
    xpsRBTFreeNode(treePtr, root);

    return data;
}

XP_STATUS xpsRBTUpdateMemoryAddress(xpsRBTree_t* treePtr)
{
    XP_STATUS ret = XP_NO_ERR;
    xpsRBNode_t* curr = NULL;

    if (!(treePtr->root))
    {
        printf("%s: No nodes in the tree \n", __FUNCTION__);
        return ret;
    }
    curr = treePtr->root;

    while (curr)
    {
        if (!(curr->isVisitAndConvert))
        {
            if ((ret = xpUpdateAddress((void **)&(curr->llink))) != XP_NO_ERR)
            {
                return ret;
            }
            if ((ret = xpUpdateAddress((void **)&(curr->rlink))) != XP_NO_ERR)
            {
                return ret;
            }
            if ((ret = xpUpdateAddress((void **)&(curr->parent))) != XP_NO_ERR)
            {
                return ret;
            }
            if ((ret = xpUpdateAddress((void **)&(curr->data))) != XP_NO_ERR)
            {
                return ret;
            }
            curr->isVisitAndConvert |= XP_RBT_NODE_ADDR_CONVERT;
        }
        if (curr->llink && (!(curr->llink->isVisitAndConvert & XP_RBT_NODE_VISIT)))
        {
            curr = curr->llink;
            continue;
        }
        if (curr->rlink && (!(curr->rlink->isVisitAndConvert & XP_RBT_NODE_VISIT)))
        {
            curr = curr->rlink;
            continue;
        }
        if (!(curr->isVisitAndConvert & XP_RBT_NODE_VISIT))
        {
            curr->isVisitAndConvert = XP_RBT_NODE_VISIT;
        }
        if (curr->llink)
        {
            curr->llink->isVisitAndConvert &= ~(XP_RBT_NODE_VISIT);
        }
        if (curr->rlink)
        {
            curr->rlink->isVisitAndConvert &= ~(XP_RBT_NODE_VISIT);
        }

        curr = curr->parent;
    }
    if (treePtr->root)
    {
        treePtr->root->isVisitAndConvert = XP_RBT_NODE_CLEAR_ALL;
    }

    return ret;
}

void* xpsRBTGetInorderSuccessor(xpsRBTree_t* treePtr, void* key)
{
    xpsRBNode_t *node=NULL;

    if (treePtr == NULL)
    {
        return NULL;
    }

    if (key != NULL)
    {
        node = xpsRBTSearchNodePtr(treePtr->root, treePtr->comp, key);
        /* key not found. */
        if (node == NULL)
        {
            return NULL;
        }
    }
    /* node of the key or NULL(to get first node) */
    node = xpsGetNextInorderNode(treePtr, node);

    /* no inorder succesor */
    if (node == NULL)
    {
        return NULL;
    }

    return node->data;
}


/* Private functions */
static xpsRBNode_t* xpsRBTSearchNodePtr(xpsRBNode_t *root, xpsComp_t comp,
                                        void* key)
{
    void *data = NULL;

    if (key == NULL)
    {
        return NULL;
    }

    while (root != NULL)
    {
        if (root->data != NULL)
        {
            data = root->data;
        }
        else
        {
            return NULL;
        }

        if ((*comp)(key, data) < 0)
        {
            root=root->llink;
        }
        else if ((*comp)(key, data) > 0)
        {
            root=root->rlink;
        }
        else
        {
            return root;
        }
    }
    return root;
}

static xpsRBNode_t * xpsGetNextInorderNode(xpsRBTree_t* treePtr,
                                           xpsRBNode_t *node)
{
    if (node == NULL)
    {
        /* First node in inorder */
        node = treePtr->root;
        if (node == NULL)
        {
            /* No node in tree. return NULL. end of traversal.*/
            return NULL;
        }
        /* leftmost node. */
        while (node->llink != NULL)
        {
            node = node->llink;
        }
        return node;
    }

    if (node->rlink != NULL)
    {
        return (xpsRBTGetMinPtr(node->rlink));
    }

    do
    {
        if (node->parent == NULL)
        {
            /* root node + no rlink. End of traversal */
            return NULL;
        }

        if (node == node->parent->rlink)
        {
            node = node->parent;
        }
        else
        {
            break;
        }
    }
    while (1);

    return (node->parent);
}

static xpsRBNode_t* xpsRBTGetMinPtr(xpsRBNode_t *root)
{
    if (root == NULL)
    {
        return NULL;
    }

    while (root->llink != NULL)
    {
        root = root->llink;
    }

    return root;
}

static void xpsRBTFreeNode(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    if (node != NULL)
    {
        XP_PERSISTENT_FREE(node);
        (treePtr->count)--;
    }
}

static xpsRBNode_t* xpsRBTGetNewNode(xpsRBTree_t *treePtr, void *data)
{
    xpsRBNode_t *tmp=NULL;

    tmp = (xpsRBNode_t*)XP_PERSISTENT_MALLOC(sizeof(xpsRBNode_t));
    if (tmp == NULL)
    {
        return NULL;
    }

    tmp->llink = NULL;
    tmp->rlink = NULL;
    tmp->parent = NULL;
    tmp->color = RED;
    tmp->data = data;
    tmp->isVisitAndConvert = XP_RBT_NODE_CLEAR_ALL;

    (treePtr->count)++;
    return tmp;
}

static xpsRBNode_t* xpsRBTGetGrandParent(xpsRBNode_t* node)
{
    if ((node != NULL) && (node->parent !=NULL))
    {
        return node->parent->parent;
    }
    else
    {
        return NULL;
    }
}

static xpsRBNode_t* xpsRBTGetUncle(xpsRBNode_t* node)
{
    xpsRBNode_t* gp = NULL;
    gp = xpsRBTGetGrandParent(node);
    if (gp == NULL)
    {
        return NULL;
    }

    if (node->parent == gp->rlink)
    {
        return (gp->llink);
    }
    else
    {
        return (gp->rlink);
    }
}

static void xpsRBTRotateLeft(xpsRBTree_t* treePtr, xpsRBNode_t* node)
{
    xpsRBNode_t *rightChild = node->rlink;
    xpsRBNode_t *parent = node->parent;

    node->rlink = rightChild->llink;
    if (node->rlink != NULL)
    {
        node->rlink->parent = node;
    }

    rightChild->llink = node;
    node->parent = rightChild;

    rightChild->parent = parent;

    if (parent==NULL)
    {
        /* set the right child root */
        treePtr->root = rightChild;
    }
    else
    {
        if (parent->llink == node)
        {
            parent->llink = rightChild;
        }
        else
        {
            parent->rlink = rightChild;
        }
    }
}

/*
   @comments Rotates the node to right
   @params
   @return
 */
static void xpsRotateRight(xpsRBTree_t* treePtr, xpsRBNode_t* node)
{
    xpsRBNode_t *leftChild = node->llink;
    xpsRBNode_t *parent = node->parent;

    node->llink = leftChild->rlink;

    if (node->llink != NULL)
    {
        node->llink->parent = node;
    }

    leftChild->rlink = node;
    node->parent = leftChild;

    leftChild->parent = parent;
    if (parent==NULL)
    {
        /* set the left child root */
        treePtr->root = leftChild;
    }
    else
    {
        if (parent->llink == node)
        {
            parent->llink = leftChild;
        }
        else
        {
            parent->rlink = leftChild;
        }
    }
}

/*
   @comments
   Orientation of nodes(node,p,gp) is r,r,r or l,l,l.
    Colors and call the appropriate rotate()
   @params
   @return
 */
static void xpsRBTColorRotate(xpsRBTree_t* treePtr, xpsRBNode_t* node)
{
    xpsRBNode_t *parent=NULL, *gp=NULL;

    parent = node->parent;
    gp = xpsRBTGetGrandParent(node);

    if (!parent || !gp)
    {
        return;
    }

    parent->color = BLACK;
    gp->color = RED;

    if ((node==parent->llink) && (parent==gp->llink))
    {
        xpsRotateRight(treePtr, gp);
    }

    if ((node==parent->rlink) && (parent==gp->rlink))
    {
        xpsRBTRotateLeft(treePtr, gp);
    }
}

/*
   @comments Rotates Left right, right-left orientation
             to RRR,LLL orientation.
   @params
   @return
 */
static void xpsRBTRotate(xpsRBTree_t* treePtr, xpsRBNode_t* node)
{
    xpsRBNode_t *parent = NULL, *gp = NULL;
    //xpsRBNode_t* uncle = NULL, *parent = NULL, *gp = NULL;

    parent = node->parent;

    /* If parent are black => No fix needed */
    if ((parent != NULL) && (parent->color != RED))
    {
        return;
    }

    //uncle = xpsRBTGetUncle(node);
    if (parent != NULL)
    {
        gp = parent->parent;
    }

    /* uncle blk , can be null */
    /*left-right case*/
    if ((gp == NULL) && (parent != NULL))
    {
        parent->color = BLACK;
        return;
    }

    /* left right case. */
    if ((parent != NULL) && (node == parent->rlink) && (parent == gp->llink))
    {
        xpsRBTRotateLeft(treePtr, parent);
        node = node->llink;
    }

    /* right-left case*/
    else if ((parent != NULL) && (node == parent->llink) && (parent == gp->rlink))
    {
        xpsRotateRight(treePtr, parent);
        node = node->rlink;
    }
    /* color rotate (rightright, leftleft) */
    xpsRBTColorRotate(treePtr, node);
}

/*
   @comments
     If tree violated red-black properties then fix it.
     New node which is added is always red, if parent is red then
     it violates RB tree property.
     1. Uncle is red
     2. Uncle is black(rotations are involved)
   @params
   @return
 */
static void xpsRBTFixRedBlackTree(xpsRBTree_t* treePtr, xpsRBNode_t* node)
{
    xpsRBNode_t* uncle = NULL, *parent = NULL, *gp = NULL;

    parent = node->parent;

    /* If parent are black => No fix needed */
    if ((parent != NULL) && (parent->color != RED))
    {
        return;
    }

    uncle = xpsRBTGetUncle(node);
    if (parent != NULL)
    {
        gp = parent->parent;
    }

    /* case : Parent and uncle are red nodes */
    if ((uncle != NULL) && (uncle->color == RED) && (parent != NULL) &&
        (gp != NULL))
    {
        xpsRBNode_t* tmpUncle = uncle, *tmpParent = parent;
        xpsRBNode_t *tmpGp = gp, *tmpNode = node;

        do
        {
            /* Fix the color issue at that node. */

            tmpUncle->color = BLACK;
            tmpParent->color = BLACK;
            tmpGp->color = RED;

            /* The color issue might have move two generations
             * So run the same coloring until we reach root or
             * we have black uncle.
             * Black uncle=> rotation & color issues are fixed.
             * If root is RED the change it to BLACK. */
            tmpNode = tmpGp;
            tmpParent = tmpNode->parent;
            /* parent is black. no issue with RB properties*/
            if (tmpParent == NULL)
            {
                /*we reached root*/
                tmpNode->color = BLACK;
                return;
            }
            else if (tmpParent->color == BLACK)
            {
                /* parent is black, node is red we have corrected black height */
                return;
            }

            tmpGp = xpsRBTGetGrandParent(tmpNode);
            if (tmpGp == NULL)
            {
                /* parent is the root.*/
                tmpParent->color = BLACK;
                return;
            }
            tmpUncle = xpsRBTGetUncle(tmpNode);
            if ((tmpUncle == NULL) || (tmpUncle->color == BLACK))
            {
                /* parent is RED, uncle is BLACK. */
                xpsRBTRotate(treePtr, tmpNode);
                return;
            }
        }
        while (1);
    }

    xpsRBTRotate(treePtr, node);
}


/*
 * Changes the links acoording color of tree. If succesor(minnode)
 * is black, and its child is a leaf node, a dummy node links are set.
 * Links are corrected after correction.
 *
 * returns:- pointer to successor(minNode), its child
 */
static  xpsRBNode_t* xpsRBTGetMin(xpsRBTree_t *treePtr, xpsRBNode_t *root,
                                  xpsRBNode_t **delSuccesor, xpsRBNode_t* dummyNode)
{
    xpsRBNode_t *minNode = NULL;

    if (root == NULL)
    {
        return NULL;
    }

    while (root->llink != NULL)
    {
        root = root->llink;
    }

    minNode = root;

    if (minNode->color == BLACK)
    {
        /* fix the links */
        if (minNode->rlink != NULL)
        {
            xpsRBTSetParentDel(treePtr, minNode, minNode->rlink);
            *delSuccesor = minNode->rlink;
        }
        else
        {
            xpsRBTSetParentDel(treePtr, minNode, dummyNode);
            dummyNode->color = BLACK;
            *delSuccesor = dummyNode;
        }

        /* add extra blk color */
        if ((*delSuccesor)->color == RED)
        {
            (*delSuccesor)->color = RED_BLACK;
        }
        else
        {
            (*delSuccesor)->color = BLACK_BLACK;
        }
    }
    else
    {
        /* color is red=> dummy not used. Set link if minNode rlink exist.*/
        xpsRBTSetParentDel(treePtr, minNode, minNode->rlink);
        *delSuccesor = minNode->rlink;
    }
    return minNode;
}

/*
   @comments return sibling or null.
   @params
   @return
 */
static xpsRBNode_t * xpsRBTGetSibling(xpsRBNode_t *node)
{
    if (node->parent == NULL)
    {
        return NULL;
    }

    if (node->parent->rlink == node)
    {
        return node->parent->llink;
    }

    return node->parent->rlink;
}

/* node's(left node) sibling is RED
 * parent & sibling is not NULL. */
static xpsRBNode_t * xpsRBTDelCase1L(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsColor_e clr;
    xpsRBNode_t *parent = node->parent, *gp;

    /*red sibling becomes red gp of node after roatation.*/
    xpsRBTRotateLeft(treePtr, parent);
    gp=parent->parent;

    /* swap colors of parent & gp.*/
    clr = parent->color;
    parent->color = gp->color;
    gp->color = clr;

    return node;
}

/*
   @comments Symmetric to case1l.
   @params
   @return
 */
static xpsRBNode_t * xpsRBTDelCase1R(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsRBNode_t *parent = node->parent, *gp;
    xpsColor_e clr;

    xpsRotateRight(treePtr, parent);

    gp=parent->parent;

    /* swap colors of parent & gp.*/
    clr = parent->color;
    parent->color = gp->color;
    gp->color = clr;

    return node;
}

/*
   @comments
    =>Assumption Dummy node is added to represent a leaf node
    which is colored BLACK_BLACK. It's data pointer is NULL.
   @params
   @return
 */
static void xpsRBTRemoveExtraBNode(xpsRBNode_t *node)
{
    if (node->data == NULL)
    {
        /*dummy node- reset parent link to NULL*/
        if (node->parent->rlink == node)
        {
            node->parent->rlink = NULL;
        }
        else
        {
            node->parent->llink = NULL;
        }
    }
    else
    {
        if (node->color == RED_BLACK)
        {
            node->color = RED;
        }
        else
        {
            node->color = BLACK;
        }
    }
}

/*
   @comments
        node's(left node) sibling and its children are black.
        Same code work for mirror image also
   @params
   @return
 */
static xpsRBNode_t * xpsRBTDelCase2(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsRBNode_t *sib;
    sib = xpsRBTGetSibling(node);
    xpsRBTRemoveExtraBNode(node);

    if (sib)
    {
        sib->color = RED;
    }

    /* Add the extra black which was removed from both children
     * to parent & fix parent. */
    if (node->parent->color == RED)
    {
        node->parent->color = BLACK;
    }
    else
    {
        node->parent->color = BLACK_BLACK;
    }

    return node->parent;
}

/*
   @comments node sibling is black, with its left link red &
             right link black.
   @params
   @return
 */
static xpsRBNode_t * xpsRBTDelCase3L(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsColor_e clr;
    xpsRBNode_t *sib, *sibRot;

    sib = xpsRBTGetSibling(node);
    if (sib)
    {
        xpsRotateRight(treePtr, sib);
        sibRot = xpsRBTGetSibling(node);
        if (sibRot)
        {
            /* swap colors of sib sibRot.*/
            clr = sib->color;
            sib->color = sibRot->color;
            sibRot->color = clr;
        }
    }

    /* continue to case 4 */
    return node;
}

/*
   @comments Symmetric to case 3L
   @params
   @return
 */
static xpsRBNode_t * xpsRBTDelCase3R(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsColor_e clr;
    xpsRBNode_t *sib, *sibRot;
    sib = xpsRBTGetSibling(node);

    if (sib)
    {
        xpsRBTRotateLeft(treePtr, sib);

        sibRot = xpsRBTGetSibling(node);

        if (sibRot)
        {
            /* swap colors of sib sibRot.*/
            clr = sib->color;
            sib->color = sibRot->color;
            sibRot->color = clr;
        }
    }

    /* continue to case 4 */
    return node;
}

/*
   @comments  node sibling is black with right child red.
   @params
   @return
 */
static void xpsRBTDelCase4L(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsRBNode_t *parent = node->parent, *uncle, *gp;

    xpsRBTRotateLeft(treePtr, parent);

    gp = parent->parent;
    gp->color = parent->color;
    parent->color = BLACK;
    xpsRBTRemoveExtraBNode(node);

    uncle = xpsRBTGetUncle(node);
    if (uncle)
    {
        uncle->color = BLACK;
    }
    return;
}

/*
   @comments Symmetric as case 4L
   @params
   @return
 */
static void xpsRBTDelCase4R(xpsRBTree_t *treePtr, xpsRBNode_t *node)
{
    xpsRBNode_t *parent = node->parent, *uncle, *gp;
    xpsRotateRight(treePtr, parent);

    gp = parent->parent;
    gp->color = parent->color;
    xpsRBTRemoveExtraBNode(node);
    parent->color = BLACK;

    uncle = xpsRBTGetUncle(node);
    if (uncle)
    {
        uncle->color = BLACK;
    }
    return;
}

/*
   @comments
   @params
   @return
 */
static uint8_t xpsRBTIsBlack(xpsRBNode_t *node)
{
    if (node == NULL)
    {
        return 1;
    }
    else
    {
        if (node->color == BLACK)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

/*
   @comments Decides the  delete case and returns the enum
   @params
   @return
 */
static xpsDelCase_e xpsRBTGetDelCase(xpsRBNode_t *node)
{
    xpsRBNode_t *sib = xpsRBTGetSibling(node);
    /* decide orientation */
    if (node->parent == NULL)
    {
        return CASE5;
    }
    if (node->parent->llink == node)
    {
        /* Extra BLACK node on left side. */
        /* RED sibling */
        if (!xpsRBTIsBlack(sib))
        {
            return CASE1L;
        }
        else
        {
            /* sibling BLACK */
            if (xpsRBTIsBlack(sib->llink) && xpsRBTIsBlack(sib->rlink))
            {
                return CASE2;
            }
            if (!xpsRBTIsBlack(sib->llink) && xpsRBTIsBlack(sib->rlink))
            {
                return CASE3L;
            }
            if (!xpsRBTIsBlack(sib->rlink))
            {
                return CASE4L;
            }
        }
    }
    else
    {
        /* Extra BLACK node on right side. */
        /* RED sibling */
        if (!xpsRBTIsBlack(sib))
        {
            return CASE1R;
        }
        else
        {
            /* sibling BLACK */
            if (xpsRBTIsBlack(sib->llink) && xpsRBTIsBlack(sib->rlink))
            {
                return CASE2;
            }
            if (xpsRBTIsBlack(sib->llink) && !xpsRBTIsBlack(sib->rlink))
            {
                return CASE3R;
            }
            if (!xpsRBTIsBlack(sib->llink))
            {
                return CASE4R;
            }
        }
    }

    return CASE_INVALID;
}

/* node has an extra black.
 * if node->data is NULL then dummyNode is used.
 * fix parent pointing to dummyNode to null */
static void xpsRBTDelFixRBTree(xpsRBTree_t *treePtr, xpsRBNode_t *n)
{
    xpsDelCase_e ch;
    xpsRBNode_t *node = n;

    while (1)
    {
        ch = xpsRBTGetDelCase(node);
        switch (ch)
        {
            case CASE1L:
                node = xpsRBTDelCase1L(treePtr, node);
                break;
            case CASE1R:
                node = xpsRBTDelCase1R(treePtr, node);
                break;
            case CASE2:
                node = xpsRBTDelCase2(treePtr, node);
                if (node->color == BLACK)
                {
                    return;
                }
                else
                {
                    break;
                }

            case CASE3L:
                node = xpsRBTDelCase3L(treePtr, node);
                break;
            case CASE3R:
                node = xpsRBTDelCase3R(treePtr, node);
                break;
            case CASE4L:
                xpsRBTDelCase4L(treePtr, node);
                return;
            case CASE4R:
                xpsRBTDelCase4R(treePtr, node);
                return;
            case CASE5:
                node->color = BLACK;
                return;
            default:
                //      LOGFN("%s: %d Unknown delete Case - Fix \n",__FUNCTION__,__LINE__);
                return;
        }
    }
}

/*
   @comments
   @params
   @return - node->parent becomes child's parent.
   */
static void xpsRBTSetParentDel(xpsRBTree_t *treePtr, xpsRBNode_t *node,
                               xpsRBNode_t *child)
{
    if (node->parent != NULL)
    {
        if (node->parent->rlink == node)
        {
            node->parent->rlink = child;
        }
        else
        {
            node->parent->llink = child;
        }
    }
    else
    {
        /* root in tree structure should be updated. */
        treePtr->root = child;
    }

    if (child != NULL)
    {
        child->parent = node->parent;
    }
}

#ifdef __cplusplus
}
#endif

