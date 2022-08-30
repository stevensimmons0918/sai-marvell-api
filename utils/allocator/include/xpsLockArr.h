// xpsLockArr.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _INCLUDE_XPSAL_XPLOCKARRAY_
#define _INCLUDE_XPSAL_XPLOCKARRAY_

#include "xpsLock.h"

#include <vector>

#define XP_LOCK_DISABLE_VALUE     ((xpsGenLock *)0xdeadbeef)

/* System lock macros (Never get disabled) */
#define XP_SYS_LOCK_CREATE(dev, index, num)     xpsLockArray::instance()->createLock((xpDevice_t)(dev), (uint32_t)(index), (uint32_t)(num))
#define XP_SYS_LOCK_DESTROY(dev, index)         xpsLockArray::instance()->destroyLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_SYS_LOCK_ALLOCATE(devId, lockId)     xpsLockArray::instance()->allocateLockId((xpDevice_t)(devId), (lockId));
#define XP_SYS_LOCK_GET(dev, index)             xpsLockArray::instance()->getLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_SYS_LOCK_READ_PROTECT(...)           xpsLockRdAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_SYS_LOCK_WRITE_PROTECT(...)          xpsLockWrAuto lock(__FUNCTION__, __VA_ARGS__)
#ifndef XP_MT_ENABLED
#define XP_SYS_MT_EXCLUDE_LOCK_CREATE(dev, index, num)     xpsLockArray::instance()->createLock((xpDevice_t)(dev), (uint32_t)(index), (uint32_t)(num))
#define XP_SYS_MT_EXCLUDE_LOCK_DESTROY(dev, index)         xpsLockArray::instance()->destroyLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_SYS_MT_EXCLUDE_LOCK_ALLOCATE(devId, lockId)     xpsLockArray::instance()->allocateLockId((xpDevice_t)(devId), (lockId));
#define XP_SYS_MT_EXCLUDE_LOCK_GET(dev, index)             xpsLockArray::instance()->getLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_SYS_MT_EXCLUDE_LOCK_READ_PROTECT(...)           xpsLockRdAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_SYS_MT_EXCLUDE_LOCK_WRITE_PROTECT(...)          xpsLockWrAuto lock(__FUNCTION__, __VA_ARGS__)
#else
#define XP_SYS_MT_EXCLUDE_LOCK_CREATE(dev, index, num)     do {} while(0)
#define XP_SYS_MT_EXCLUDE_LOCK_DESTROY(dev, index)         do {} while(0)
#define XP_SYS_MT_EXCLUDE_LOCK_ALLOCATE(devId, lockId)     do {} while(0)
#define XP_SYS_MT_EXCLUDE_LOCK_GET(dev, index)             do {} while(0)
#define XP_SYS_MT_EXCLUDE_LOCK_READ_PROTECT(...)           do {} while(0)
#define XP_SYS_MT_EXCLUDE_LOCK_WRITE_PROTECT(...)          do {} while(0)
#endif

#define XP_LOCK_DISABLE_VALUE     ((xpsGenLock *)0xdeadbeef)

/* General lock macros */
#ifdef XP_MT_ENABLED
#define XP_RW_LOCK_CREATE(dev, index, num)      xpsLockArray::instance()->createLock((xpDevice_t)(dev), (uint32_t)(index), (uint32_t)(num))
#define XP_RW_LOCK_DESTROY(dev, index)          xpsLockArray::instance()->destroyLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_RW_LOCK_ALLOCATE(devId, lockId)      xpsLockArray::instance()->allocateLockId(devId, lockId);
#define XP_RW_LOCK_READ_PROTECT(...)            xpsLockRdAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_RW_LOCK_WRITE_PROTECT(...)           xpsLockWrAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_RW_LOCK_READ_PROTECT_ORDERED(...)    xpsLockOrderedRdAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_RW_LOCK_WRITE_PROTECT_ORDERED(...)   xpsLockOrderedWrAuto lock(__FUNCTION__, __VA_ARGS__)
#define XP_RW_LOCK_ENABLE(dev, index)           xpsLockArray::instance()->enableLock((xpDevice_t)(dev), (uint32_t)(index))
#define XP_RW_LOCK_DISABLE(dev, index)          xpsLockArray::instance()->disableLock((xpDevice_t)(dev), (uint32_t)(index))
#else
#define XP_RW_LOCK_CREATE(dev, index, num)      XP_LOCK_DISABLE_VALUE
#define XP_RW_LOCK_DESTROY(dev, index)          do {} while(0)
#define XP_RW_LOCK_ALLOCATE(devId, lockId)      do {} while(0)
#define XP_RW_LOCK_READ_PROTECT(...)            do {} while(0)
#define XP_RW_LOCK_WRITE_PROTECT(...)           do {} while(0)
#define XP_RW_LOCK_READ_PROTECT_ORDERED(...)    do {} while(0)
#define XP_RW_LOCK_WRITE_PROTECT_ORDERED(...)   do {} while(0)
#define XP_RW_LOCK_ENABLE(dev, index)           do {} while(0)
#define XP_RW_LOCK_DISABLE(dev, index)          do {} while(0)
#endif

/* PL locks macros */
#ifdef XP_PL_LOCK_ENABLED
#define XP_RW_PL_LOCK_CREATE(dev, index, num)      XP_RW_LOCK_CREATE(dev, index, num)
#define XP_RW_PL_LOCK_DESTROY(dev, index)          XP_RW_LOCK_DESTROY(dev, index)
#define XP_RW_PL_LOCK_READ_PROTECT(...)            XP_RW_LOCK_READ_PROTECT(__VA_ARGS__)
#define XP_RW_PL_LOCK_WRITE_PROTECT(...)           XP_RW_LOCK_WRITE_PROTECT(__VA_ARGS__)
#else
#define XP_RW_PL_LOCK_CREATE(dev, index, num)      XP_LOCK_DISABLE_VALUE
#define XP_RW_PL_LOCK_DESTROY(dev, index)          do {} while(0)
#define XP_RW_PL_LOCK_READ_PROTECT(...)            do {} while(0)
#define XP_RW_PL_LOCK_WRITE_PROTECT(...)           do {} while(0)
#endif

/* FL locks macros */
#ifdef XP_FL_LOCK_ENABLED
#define XP_RW_FL_LOCK_CREATE(dev, index, num)      XP_RW_LOCK_CREATE(dev, index, num)
#define XP_RW_FL_LOCK_DESTROY(dev, index)          XP_RW_LOCK_DESTROY(dev, index)
#define XP_RW_FL_LOCK_READ_PROTECT(...)            XP_RW_LOCK_READ_PROTECT(__VA_ARGS__)
#define XP_RW_FL_LOCK_WRITE_PROTECT(...)           XP_RW_LOCK_WRITE_PROTECT(__VA_ARGS__)
#define XP_RW_FL_LOCK_READ_PROTECT_ORDERED(...)    XP_RW_LOCK_READ_PROTECT_ORDERED(__VA_ARGS__)
#define XP_RW_FL_LOCK_WRITE_PROTECT_ORDERED(...)   XP_RW_LOCK_WRITE_PROTECT_ORDERED(__VA_ARGS__)
#else
#define XP_RW_FL_LOCK_CREATE(dev, index, num)      XP_LOCK_DISABLE_VALUE
#define XP_RW_FL_LOCK_DESTROY(dev, index)          do {} while(0)
#define XP_RW_FL_LOCK_READ_PROTECT(...)            do {} while(0)
#define XP_RW_FL_LOCK_WRITE_PROTECT(...)           do {} while(0)
#define XP_RW_FL_LOCK_READ_PROTECT_ORDERED(...)    do {} while(0)
#define XP_RW_FL_LOCK_WRITE_PROTECT_ORDERED(...)   do {} while(0)
#endif


#ifdef __cplusplus

/**
 * \class xpsLockArray
 * \brief Class to manage XDK lock array.
 *
 * This class contains range of locks required by XDK, implements
 * routines to handle general system locks, and provides lock create
 * and destroy functionality.
 */
class xpsLockArray
{
    xpsGenLock
    lock;                                           /* lock for this class' integrity */
    xpsGenLock
    *rangeStatic[XP_MAX_DEVICES][XP_LOCKINDEX_MAX]; /* array of xpsGenLock pointer associated
                                                               * with lock identifiers from xpLockIndex_e */
    std::vector<xpsGenLock *>
    rangeDynamic[XP_MAX_DEVICES];         /* array of xpsGenLock pointer associated
                                                               * with dynamically allocated lock identifiers */
    std::vector<uint32_t>
    dynId[XP_MAX_DEVICES];                   /* array of dynamically allocated lock identifiers */

    /**
     * \public
     * \brief Constructor of xpsLockArray
     *
     */
    xpsLockArray();

    /**
     * \public
     * \brief Destructor of xpsLockArray
     *
     */
    ~xpsLockArray();

    /**
     * \public
     * \brief Create locks required on pre-init stage
     *
     */
    void createGenSystemLocks(void);

    /**
     * \public
     * \brief Destroy locks required on pre-init stage
     *
     */
    void destroyGenSystemLocks(void);

public:

    /**
     * \public
     * \brief Get instance of xpsLockArray.
     *
     * The xpsLockArray is the singleton class. Initially the value of variable
     * will be NULL. On first call to this method, new instance is created.
     * For all subsequent calls the same instance is returned.
     *
     * \return xpsLockArray*
     */
    static xpsLockArray* instance();

    /**
     * \public
     * \brief Destroy the instance of xpsLockArray.
     *
     * The xpsLockArray is the singleton class. This function must be called
     * during shutdown to correctly destroy xpsLockArray instance and clean up
     * used resources.
     *
     * \return none
     */
    static void destroy();

    /*
     * \public
     * print info about allocated locks
     *
     */
    void printLockInfo();

    /**
     * \public
     * \brief enable all logs per lock idx
     *
     * xpsGenLock by default has disabled logs.
     *
     * \return int
     */
    int enableAllLockLogs();

    /**
     * \public
     * \brief enable all logs per lock idx
     *
     * xpsGenLock by default has disabled logs.
     *
     * \return int
     */
    int disableAllLockLogs();

    /**
     * \public
     * \brief Allocate new lock identifier.
     *
     * xpsLockArray operates with either with predefined set of lock identifiers
     * defined in xpLockIndex_e or identifiers allocated in runtime. This
     * method allocates new lock identifier in runtime.
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return int
     */
    int allocateLockId(xpDevice_t devId, uint32_t &lockId);

    /**
     * \public
     * \brief Release lock identifier.
     *
     * This method release lock identifier previously allocated by
     * allocateLockId method
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return int
     */
    int releaseLockId(xpDevice_t devId, uint32_t lockId);

    /**
     * \public
     * \brief Creates new xpsGenLock object for lock identifier.
     *
     * This method constructs new xpsGenLock object with all underlying
     * infrastructure for provided lock identifier.
     *
     * \param [in] devId
     * \param [in] lockId
     * \param [in] mutexNum
     *
     * \return xpsGenLock*
     */
    xpsGenLock* createLock(xpDevice_t devId, uint32_t lockId,
                           uint32_t mutexNum = 1);

    /**
     * \public
     * \brief Destroy xpsGenLock object for lock identifier.
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return none
     */
    void destroyLock(xpDevice_t devId, uint32_t lockId);

    /**
     * \public
     * \brief Get pointer to xpsGenLock object associated with lock identifier
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return xpsGenLock*
     */
    xpsGenLock* getLock(xpDevice_t devId, uint32_t lockId);

    /**
     * \public
     * \brief Disable lock using lock identifier
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return int
     */
    int disableLock(xpDevice_t devId, uint32_t lockId);

    /**
     * \public
     * \brief Enable lock using lock identifier
     *
     * \param [in] devId
     * \param [in] lockId
     *
     * \return int
     */
    int enableLock(xpDevice_t devId, uint32_t lockId);
};

/**
 * \class xpsLockRdAuto
 * \brief Implementation of read lock guard class.
 *
 */
class xpsLockRdAuto
{
private:
    xpsGenLock* lock;
    uint32_t range;

    xpsLockRdAuto();
    xpsLockRdAuto(const xpsLockRdAuto&);
    xpsLockRdAuto operator=(xpsLockRdAuto);
public:
    /**
     * \public
     * \brief Guard class implementation
     *
     * Enter protected section for a particular lockId
     *
     * \param [in] lockOwner
     * \param [in] pLock
     * \param [in] mutexId
     *
     */
    xpsLockRdAuto(const char *lockOwner, xpsGenLock* pLock, uint32_t mutexId = 0);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Enter protected section for a particular lockId
     *
     * \param [in] lockOwner
     * \param [in] devId
     * \param [in] lockId
     * \param [in] mutexId
     *
     */
    xpsLockRdAuto(const char *lockOwner, xpDevice_t devId, uint32_t lockId,
                  uint32_t mutexId = 0);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Exit protected section
     *
     */
    ~xpsLockRdAuto();
};

/**
 * \class xpsLockWrAuto
 * \brief Implementation of write lock guard class.
 *
 */
class xpsLockWrAuto
{
private:
    xpsGenLock* lock;
    uint32_t range;

    xpsLockWrAuto();
    xpsLockWrAuto(const xpsLockWrAuto&);
    xpsLockWrAuto operator=(xpsLockWrAuto);
public:
    /**
     * \public
     * \brief Guard class implementation
     *
     * Enter protected section for a particular lockId
     *
     * \param [in] lockOwner
     * \param [in] pLock
     * \param [in] mutexId
     *
     */
    xpsLockWrAuto(const char *lockOwner, xpsGenLock* pLock, uint32_t mutexId = 0);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Enter protected section for a particular lockId
     *
     * \param [in] lockOwner
     * \param [in] devId
     * \param [in] lockId
     * \param [in] mutexId
     *
     */
    xpsLockWrAuto(const char *lockOwner, xpDevice_t devId, uint32_t lockId,
                  uint32_t mutexId = 0);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Exit protected section
     *
     */
    ~xpsLockWrAuto();
};

/**
 * \class xpsLockOrderedWrAuto
 * \brief Implementation of write lock ordered guard class.
 *
 * Ordered write lock guard class implementation. This class provides
 * functionality to lock/unlock multiple critical sections simultaneously.
 * Lock get/put actions are performed in order.
 */
class xpsLockOrderedWrAuto
{
    std::vector<xpsGenLock *> lock_lst;
public:
    /**
     * \public
     * \brief Guard class implementation for ordered locking
     *
     * Enter protected section for a particular lockId's in ordered way
     *
     * \param [in] lockOwner
     * \param [in] lockNum
     *
     */
    xpsLockOrderedWrAuto(const char *lockOwner, int lockNum, ...);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Exit protected section
     *
     */
    ~xpsLockOrderedWrAuto();
};

/**
 * \class xpsLockOrderedRdAuto
 * \brief Implementation of read lock ordered guard class.
 *
 * Ordered write lock guard class implementation. This class provides
 * functionality to lock/unlock multiple critical sections simultaneously.
 * Lock get/put actions are performed in order.
 */
class xpsLockOrderedRdAuto
{
    std::vector<xpsGenLock *> lock_lst;
public:
    /**
     * \public
     * \brief Guard class implementation for ordered locking
     *
     * Enter protected section for a particular lockId's in ordered way
     *
     * \param [in] lockOwner
     * \param [in] lockNum
     *
     */
    xpsLockOrderedRdAuto(const char *lockOwner, int lockNum, ...);
    /**
     * \public
     * \brief Guard class implementation
     *
     * Exit protected section
     *
     */
    ~xpsLockOrderedRdAuto();
};

class xpsLockMgr
{
protected:
    xpsGenLock* xpMgrLock[XP_MAX_DEVICES];

public:
    xpsLockMgr();
    virtual ~xpsLockMgr();
    virtual xpsGenLock* getLock(xpDevice_t devId);
};

#endif /* __cplusplus */
#endif
