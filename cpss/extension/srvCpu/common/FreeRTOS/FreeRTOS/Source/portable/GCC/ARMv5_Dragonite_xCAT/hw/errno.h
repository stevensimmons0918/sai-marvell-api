#ifndef _ARM_ERRNO_H
#define _ARM_ERRNO_H

/* End of file character.
   Some things throughout the library rely on this being -1.  */
#ifndef EOF
# define EOF ( -1 )
#endif

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Arg list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
#define	EDEADLK		35	/* Resource deadlock would occur */
#define	ENAMETOOLONG	36	/* File name too long */
#define	ENOLCK		37	/* No record locks available */
#define	ENOSYS		38	/* Function not implemented */
#define	ENOTEMPTY	39	/* Directory not empty */
#define	ELOOP		40	/* Too many symbolic links encountered */
#define	EWOULDBLOCK	EAGAIN	/* Operation would block */
#define	ENOMSG		42	/* No message of desired type */
#define	EIDRM		43	/* Identifier removed */
#define	ECHRNG		44	/* Channel number out of range */
#define	EL2NSYNC	45	/* Level 2 not synchronized */
#define	EL3HLT		46	/* Level 3 halted */
#define	EL3RST		47	/* Level 3 reset */
#define	ELNRNG		48	/* Link number out of range */
#define	EUNATCH		49	/* Protocol driver not attached */
#define	ENOCSI		50	/* No CSI structure available */
#define	EL2HLT		51	/* Level 2 halted */
#define	EBADE		52	/* Invalid exchange */
#define	EBADR		53	/* Invalid request descriptor */
#define	EXFULL		54	/* Exchange full */
#define	ENOANO		55	/* No anode */
#define	EBADRQC		56	/* Invalid request code */
#define	EBADSLT		57	/* Invalid slot */
#define	EDEADLOCK	58	/* File locking deadlock error */
#define	EBFONT		59	/* Bad font file format */
#define	ENOSTR		60	/* Device not a stream */
#define	ENODATA		61	/* No data available */
#define	ETIME		62	/* Timer expired */
#define	ENOSR		63	/* Out of streams resources */
#define	ENONET		64	/* Machine is not on the network */
#define	ENOPKG		65	/* Package not installed */
#define	EREMOTE		66	/* Object is remote */
#define	ENOLINK		67	/* Link has been severed */
#define	EADV		68	/* Advertise error */
#define	ESRMNT		69	/* Srmount error */
#define	ECOMM		70	/* Communication error on send */
#define	EPROTO		71	/* Protocol error */
#define	EMULTIHOP	72	/* Multihop attempted */
#define	EDOTDOT		73	/* RFS specific error */
#define	EBADMSG		74	/* Not a data message */
#define	EOVERFLOW	75	/* Value too large for defined data type */
#define	ENOTUNIQ	76	/* Name not unique on network */
#define	EBADFD		77	/* File descriptor in bad state */
#define	EREMCHG		78	/* Remote address changed */
#define	ELIBACC		79	/* Can not access a needed shared library */
#define	ELIBBAD		80	/* Accessing a corrupted shared library */
#define	ELIBSCN		81	/* .lib section in a.out corrupted */
#define	ELIBMAX		82	/* Attempting to link in too many shared libraries */
#define	ELIBEXEC	83	/* Cannot exec a shared library directly */
#define	EILSEQ		84	/* Illegal byte sequence */
#define	ERESTART	85	/* Interrupted system call should be restarted */
#define	ESTRPIPE	86	/* Streams pipe error */
#define	EUSERS		87	/* Too many users */
#define	ENOTSOCK	88	/* Socket operation on non-socket */
#define	EDESTADDRREQ	89	/* Destination address required */
#define	EMSGSIZE	90	/* Message too long */
#define	EPROTOTYPE	91	/* Protocol wrong type for socket */
#define	ENOPROTOOPT	92	/* Protocol not available */
#define	EPROTONOSUPPORT	93	/* Protocol not supported */
#define	ESOCKTNOSUPPORT	94	/* Socket type not supported */
#define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */
#define	EPFNOSUPPORT	96	/* Protocol family not supported */
#define	EAFNOSUPPORT	97	/* Address family not supported by protocol */
#define	EADDRINUSE	98	/* Address already in use */
#define	EADDRNOTAVAIL	99	/* Cannot assign requested address */
#define	ENETDOWN	100	/* Network is down */
#define	ENETUNREACH	101	/* Network is unreachable */
#define	ENETRESET	102	/* Network dropped connection because of reset */
#define	ECONNABORTED	103	/* Software caused connection abort */
#define	ECONNRESET	104	/* Connection reset by peer */
#define	ENOBUFS		105	/* No buffer space available */
#define	EISCONN		106	/* Transport endpoint is already connected */
#define	ENOTCONN	107	/* Transport endpoint is not connected */
#define	ESHUTDOWN	108	/* Cannot send after transport endpoint shutdown */
#define	ETOOMANYREFS	109	/* Too many references: cannot splice */
#define	ETIMEDOUT	110	/* Connection timed out */
#define	ECONNREFUSED	111	/* Connection refused */
#define	EHOSTDOWN	112	/* Host is down */
#define	EHOSTUNREACH	113	/* No route to host */
#define	EALREADY	114	/* Operation already in progress */
#define	EINPROGRESS	115	/* Operation now in progress */
#define	ESTALE		116	/* Stale NFS file handle */
#define	EUCLEAN		117	/* Structure needs cleaning */
#define	ENOTNAM		118	/* Not a XENIX named type file */
#define	ENAVAIL		119	/* No XENIX semaphores available */
#define	EISNAM		120	/* Is a named type file */
#define	EREMOTEIO	121	/* Remote I/O error */
#define	EDQUOT		122	/* Quota exceeded */

#define	ENOMEDIUM	123	/* No medium found */
#define	EMEDIUMTYPE	124	/* Wrong medium type */

/* Should never be seen by user programs */
#define ERESTARTSYS	512
#define ERESTARTNOINTR	513
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#define ENOIOCTLCMD	515	/* No ioctl command */

#define _LAST_ERRNO	515

/* The following is a list of Marvell status    */
#define MV_ERROR			( -1 )
#define MV_OK				( 0x00 )  /* Operation succeeded                   */
#define MV_FAIL				( 0x01 )  /* Operation failed                      */
#define MV_BAD_VALUE		( 0x02 )  /* Illegal value ( general )               */
#define MV_OUT_OF_RANGE		( 0x03 )  /* The value is out of range             */
#define MV_BAD_PARAM		( 0x04 )  /* Illegal parameter in function called  */
#define MV_BAD_PTR			( 0x05 )  /* Illegal pointer value                 */
#define MV_BAD_SIZE			( 0x06 )  /* Illegal size                          */
#define MV_BAD_STATE		( 0x07 )  /* Illegal state of state machine        */
#define MV_SET_ERROR		( 0x08 )  /* Set operation failed                  */
#define MV_GET_ERROR		( 0x09 )  /* Get operation failed                  */
#define MV_CREATE_ERROR		( 0x0A )  /* Fail while creating an item           */
#define MV_NOT_FOUND		( 0x0B )  /* Item not found                        */
#define MV_NO_MORE			( 0x0C )  /* No more items found                   */
#define MV_NO_SUCH			( 0x0D )  /* No such item                          */
#define MV_TIMEOUT			( 0x0E )  /* Time Out                              */
#define MV_NO_CHANGE		( 0x0F )  /* Parameter( s ) is already in this value */
#define MV_NOT_SUPPORTED	( 0x10 )  /* This request is not support           */
#define MV_NOT_IMPLEMENTED	( 0x11 )  /* Request supported but not implemented */
#define MV_NOT_INITIALIZED	( 0x12 )  /* The item is not initialized           */
#define MV_NO_RESOURCE		( 0x13 )  /* Resource not available ( memory ... )   */
#define MV_FULL				( 0x14 )  /* Item is full ( Queue or table etc... )  */
#define MV_EMPTY			( 0x15 )  /* Item is empty ( Queue or table etc... ) */
#define MV_INIT_ERROR		( 0x16 )  /* Error occured while INIT process      */
#define MV_HW_ERROR			( 0x17 )  /* Hardware error                        */
#define MV_TX_ERROR			( 0x18 )  /* Transmit operation not succeeded      */
#define MV_RX_ERROR			( 0x19 )  /* Recieve operation not succeeded       */
#define MV_NOT_READY		( 0x1A )  /* The other side is not ready yet       */
#define MV_ALREADY_EXIST	( 0x1B )  /* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM	( 0x1C )  /* Cpu memory allocation failed.	     */
#define MV_NOT_STARTED		( 0x1D )  /* Not started yet         */
#define MV_BUSY				( 0x1E )  /* Item is busy.                         */
#define MV_TERMINATE		( 0x1F )  /* Item terminates it's work.            */
#define MV_NOT_ALIGNED		( 0x20 )  /* Wrong alignment                       */
#define MV_NOT_ALLOWED		( 0x21 )  /* Operation NOT allowed                 */
#define MV_WRITE_PROTECT	( 0x22 )  /* Write protected                       */


#define MV_INVALID	( long )( -1 )

#define MV_FALSE	0
#define MV_TRUE		( !( MV_FALSE ))
#define FALSE		( 0 )
#define TRUE		( !FALSE )

#endif
