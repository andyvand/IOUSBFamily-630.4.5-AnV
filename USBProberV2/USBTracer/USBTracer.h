/*
 * Copyright © 2009-2013 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

//—————————————————————————————————————————————————————————————————————————————
//	Includes
//—————————————————————————————————————————————————————————————————————————————

#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <spawn.h>

#include <mach/clock_types.h>
#include <mach/mach_time.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/file.h>

//#include <libutil.h>		// for reexec_to_match_kernel()
#include <mach/mach_host.h> // for host_info()

/*
 * Copyright (c) 2000-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

/* 	Copyright (c) 1997 Apple Computer, Inc.  All rights reserved.
 *
 * kdebug.h -   kernel_debug definitions
 *
 */

#ifndef BSD_SYS_KDEBUG_H
#define BSD_SYS_KDEBUG_H

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
__BEGIN_DECLS

#ifdef __APPLE_API_UNSTABLE

#include <mach/clock_types.h>
#include <stdint.h>
#if	defined(KERNEL_BUILD)
#include <kdebug.h>
#endif /* KERNEL_BUILD */

#ifdef	XNU_KERNEL_PRIVATE
#include <stdint.h>
#include <mach/branch_predicates.h>
#endif

/*
 * types of faults that vm_fault handles
 * and creates trace entries for
 */
#define DBG_ZERO_FILL_FAULT   1
#define DBG_PAGEIN_FAULT      2
#define DBG_COW_FAULT         3
#define DBG_CACHE_HIT_FAULT   4
#define DBG_NZF_PAGE_FAULT    5
#define DBG_GUARD_FAULT	      6
#define DBG_PAGEINV_FAULT     7
#define DBG_PAGEIND_FAULT     8


/* The debug code consists of the following
 *
 * ----------------------------------------------------------------------
 *|              |               |                               |Func   |
 *| Class (8)    | SubClass (8)  |          Code (14)            |Qual(2)|
 * ----------------------------------------------------------------------
 * The class specifies the higher level
 */

/* The Function qualifiers  */
#define DBG_FUNC_START		1
#define DBG_FUNC_END		2
#define DBG_FUNC_NONE		0


/* The Kernel Debug Classes  */
#define DBG_MACH		1
#define DBG_NETWORK		2
#define DBG_FSYSTEM		3
#define DBG_BSD			4
#define DBG_IOKIT		5
#define DBG_DRIVERS		6
#define DBG_TRACE           	7
#define DBG_DLIL	        8
#define DBG_SECURITY		9
#define DBG_CORESTORAGE		10
#define DBG_CG         		11
#define DBG_MISC		20
#define DBG_DYLD           	31
#define DBG_QT              	32
#define DBG_APPS            	33
#define DBG_LAUNCHD         	34
#define DBG_PERF                37
#define DBG_MIG			255

/* **** The Kernel Debug Sub Classes for Mach (DBG_MACH) **** */
#define	DBG_MACH_EXCP_KTRAP_x86	0x02	/* Kernel Traps on x86 */
#define	DBG_MACH_EXCP_DFLT	0x03	/* Data Translation Fault */
#define	DBG_MACH_EXCP_IFLT	0x04	/* Inst Translation Fault */
#define	DBG_MACH_EXCP_INTR	0x05	/* Interrupts */
#define	DBG_MACH_EXCP_ALNG	0x06	/* Alignment Exception */
#define	DBG_MACH_EXCP_UTRAP_x86	0x07	/* User Traps on x86 */
#define	DBG_MACH_EXCP_FP	0x08	/* FP Unavail */
#define	DBG_MACH_EXCP_DECI	0x09	/* Decrementer Interrupt */
#define	DBG_MACH_CHUD		0x0A	/* CHUD */
#define	DBG_MACH_EXCP_SC	0x0C	/* System Calls */
#define	DBG_MACH_EXCP_TRACE	0x0D	/* Trace exception */
#define	DBG_MACH_EXCP_EMUL	0x0E	/* Instruction emulated */
#define	DBG_MACH_IHDLR		0x10	/* Interrupt Handlers */
#define	DBG_MACH_IPC		0x20	/* Inter Process Comm */
#define	DBG_MACH_VM		0x30	/* Virtual Memory */
#define	DBG_MACH_LEAKS		0x31    /* alloc/free */
#define	DBG_MACH_SCHED		0x40	/* Scheduler */
#define	DBG_MACH_MSGID_INVALID	0x50	/* Messages - invalid */
#define DBG_MACH_LOCKS		0x60	/* new lock APIs */
#define DBG_MACH_PMAP		0x70	/* pmap */
#define DBG_MACH_MP		0x90	/* MP related */

/* Codes for Scheduler (DBG_MACH_SCHED) */
#define MACH_SCHED              0x0     /* Scheduler */
#define MACH_STACK_ATTACH       0x1     /* stack_attach() */
#define MACH_STACK_HANDOFF      0x2     /* stack_handoff() */
#define MACH_CALL_CONT          0x3     /* call_continuation() */
#define MACH_CALLOUT            0x4     /* callouts */
#define MACH_STACK_DETACH       0x5
#define MACH_MAKE_RUNNABLE      0x6     /* make thread runnable */
#define	MACH_PROMOTE            0x7	/* promoted due to resource */
#define	MACH_DEMOTE             0x8	/* promotion undone */
#define MACH_IDLE               0x9	/* processor idling */
#define MACH_STACK_DEPTH        0xa	/* stack depth at switch */
#define MACH_MOVED              0xb	/* did not use original scheduling decision */
#define MACH_FAIRSHARE_ENTER    0xc	/* move to fairshare band */
#define MACH_FAIRSHARE_EXIT     0xd	/* exit fairshare band */
#define MACH_FAILSAFE           0xe	/* tripped fixed-pri/RT failsafe */
#define MACH_BLOCK              0xf	/* thread block */
#define MACH_WAIT		0x10	/* thread wait assertion */
#define	MACH_GET_URGENCY	0x14	/* Urgency queried by platform */
#define	MACH_URGENCY		0x15	/* Urgency (RT/BG/NORMAL) communicated
* to platform
*/
#define	MACH_REDISPATCH		0x16	/* "next thread" thread redispatched */
#define	MACH_REMOTE_AST		0x17	/* AST signal issued to remote processor */

#define	MACH_SCHED_LPA_BROKEN	0x18	/* last_processor affinity broken in choose_processor */

/* Codes for pmap (DBG_MACH_PMAP) */
#define PMAP__CREATE		0x0
#define PMAP__DESTROY		0x1
#define PMAP__PROTECT		0x2
#define PMAP__PAGE_PROTECT	0x3
#define PMAP__ENTER		0x4
#define PMAP__REMOVE		0x5
#define PMAP__NEST		0x6
#define PMAP__UNNEST		0x7
#define PMAP__FLUSH_TLBS	0x8
#define PMAP__UPDATE_INTERRUPT	0x9
#define PMAP__ATTRIBUTE_CLEAR	0xa

/* **** The Kernel Debug Sub Classes for Network (DBG_NETWORK) **** */
#define DBG_NETIP	1	/* Internet Protocol */
#define DBG_NETARP	2	/* Address Resolution Protocol */
#define	DBG_NETUDP	3	/* User Datagram Protocol */
#define	DBG_NETTCP	4	/* Transmission Control Protocol */
#define	DBG_NETICMP	5	/* Internet Control Message Protocol */
#define	DBG_NETIGMP	6	/* Internet Group Management Protocol */
#define	DBG_NETRIP	7	/* Routing Information Protocol */
#define	DBG_NETOSPF	8	/* Open Shortest Path First */
#define	DBG_NETISIS	9	/* Intermediate System to Intermediate System */
#define	DBG_NETSNMP	10	/* Simple Network Management Protocol */
#define DBG_NETSOCK	11	/* Socket Layer */

/* For Apple talk */
#define	DBG_NETAARP	100	/* Apple ARP */
#define	DBG_NETDDP	101	/* Datagram Delivery Protocol */
#define	DBG_NETNBP	102	/* Name Binding Protocol */
#define	DBG_NETZIP	103	/* Zone Information Protocol */
#define	DBG_NETADSP	104	/* Name Binding Protocol */
#define	DBG_NETATP	105	/* Apple Transaction Protocol */
#define	DBG_NETASP	106	/* Apple Session Protocol */
#define	DBG_NETAFP	107	/* Apple Filing Protocol */
#define	DBG_NETRTMP	108	/* Routing Table Maintenance Protocol */
#define	DBG_NETAURP	109	/* Apple Update Routing Protocol */
#define	DBG_NETIPSEC	128	/* IPsec Protocol  */

/* **** The Kernel Debug Sub Classes for IOKIT (DBG_IOKIT) **** */
#define DBG_IOINTC			0	/* Interrupt controller */
#define DBG_IOWORKLOOP		1	/* Work from work loop */
#define DBG_IOINTES			2	/* Interrupt event source */
#define DBG_IOCLKES			3	/* Clock event source */
#define DBG_IOCMDQ			4	/* Command queue latencies */
#define DBG_IOMCURS			5	/* Memory Cursor */
#define DBG_IOMDESC			6	/* Memory Descriptors */
#define DBG_IOPOWER			7	/* Power Managerment */
#define DBG_IOSERVICE		8	/* Matching etc. */

/* **** 9-32 reserved for internal IOKit usage **** */

#define DBG_IOSTORAGE		32	/* Storage layers */
#define	DBG_IONETWORK		33	/* Network layers */
#define	DBG_IOKEYBOARD		34	/* Keyboard */
#define	DBG_IOHID		35	/* HID Devices */
#define	DBG_IOAUDIO		36	/* Audio */
#define	DBG_IOSERIAL		37	/* Serial */
#define	DBG_IOTTY		38	/* TTY layers */
#define DBG_IOSAM		39	/* SCSI Architecture Model layers */
#define DBG_IOPARALLELATA   	40	/* Parallel ATA */
#define DBG_IOPARALLELSCSI	41	/* Parallel SCSI */
#define DBG_IOSATA		42	/* Serial-ATA */
#define DBG_IOSAS		43	/* SAS */
#define DBG_IOFIBRECHANNEL	44	/* FiberChannel */
#define DBG_IOUSB		45	/* USB */
#define DBG_IOBLUETOOTH		46	/* Bluetooth */
#define DBG_IOFIREWIRE		47	/* FireWire */
#define DBG_IOINFINIBAND	48	/* Infiniband */
#define DBG_IOCPUPM		49	/* CPU Power Management */
#define DBG_IOGRAPHICS		50	/* Graphics */
#define DBG_HIBERNATE		51	/* hibernation related events */


/* Backwards compatibility */
#define	DBG_IOPOINTING		DBG_IOHID			/* OBSOLETE: Use DBG_IOHID instead */
#define DBG_IODISK			DBG_IOSTORAGE		/* OBSOLETE: Use DBG_IOSTORAGE instead */

/* **** The Kernel Debug Sub Classes for Device Drivers (DBG_DRIVERS) **** */
#define DBG_DRVSTORAGE		1	/* Storage layers */
#define	DBG_DRVNETWORK		2	/* Network layers */
#define	DBG_DRVKEYBOARD		3	/* Keyboard */
#define	DBG_DRVHID			4	/* HID Devices */
#define	DBG_DRVAUDIO		5	/* Audio */
#define	DBG_DRVSERIAL		7	/* Serial */
#define DBG_DRVSAM			8	/* SCSI Architecture Model layers */
#define DBG_DRVPARALLELATA  9	/* Parallel ATA */
#define DBG_DRVPARALLELSCSI	10	/* Parallel SCSI */
#define DBG_DRVSATA			11	/* Serial ATA */
#define DBG_DRVSAS			12	/* SAS */
#define DBG_DRVFIBRECHANNEL	13	/* FiberChannel */
#define DBG_DRVUSB			14	/* USB */
#define DBG_DRVBLUETOOTH	15	/* Bluetooth */
#define DBG_DRVFIREWIRE		16	/* FireWire */
#define DBG_DRVINFINIBAND	17	/* Infiniband */
#define DBG_DRVGRAPHICS		18  /* Graphics */
#define DBG_DRVSD		19 	/* Secure Digital */
#define DBG_DRVNAND		20	/* NAND drivers and layers */
#define DBG_SSD			21	/* SSD */

/* Backwards compatibility */
#define	DBG_DRVPOINTING		DBG_DRVHID		/* OBSOLETE: Use DBG_DRVHID instead */
#define DBG_DRVDISK			DBG_DRVSTORAGE	/* OBSOLETE: Use DBG_DRVSTORAGE instead */

/* **** The Kernel Debug Sub Classes for the DLIL Layer (DBG_DLIL) **** */
#define DBG_DLIL_STATIC 1       /* Static DLIL code */
#define DBG_DLIL_PR_MOD 2       /* DLIL Protocol Module */
#define DBG_DLIL_IF_MOD 3       /* DLIL Interface Module */
#define DBG_DLIL_PR_FLT 4       /* DLIL Protocol Filter */
#define DBG_DLIL_IF_FLT 5       /* DLIL Interface FIlter */

/* The Kernel Debug Sub Classes for File System (DBG_FSYSTEM) */
#define DBG_FSRW      1       /* reads and writes to the filesystem */
#define DBG_DKRW      2       /* reads and writes to the disk */
#define DBG_FSVN      3       /* vnode operations (inc. locking/unlocking) */
#define DBG_FSLOOOKUP 4       /* namei and other lookup-related operations */
#define DBG_JOURNAL   5       /* journaling operations */
#define DBG_IOCTL     6       /* ioctl to the disk */
#define DBG_BOOTCACHE 7       /* bootcache operations */
#define DBG_HFS       8       /* HFS-specific events; see bsd/hfs/hfs_kdebug.h */
#define DBG_EXFAT     0xE     /* ExFAT-specific events; see the exfat project */
#define DBG_MSDOS     0xF     /* FAT-specific events; see the msdosfs project */

/* The Kernel Debug Sub Classes for BSD */
#define DBG_BSD_PROC		0x01	/* process/signals related */
#define	DBG_BSD_EXCP_SC		0x0C	/* System Calls */
#define	DBG_BSD_AIO		0x0D	/* aio (POSIX async IO) */
#define DBG_BSD_SC_EXTENDED_INFO 0x0E	/* System Calls, extended info */
#define DBG_BSD_SC_EXTENDED_INFO2 0x0F	/* System Calls, extended info */


/* The Codes for BSD subcode class DBG_BSD_PROC */
#define BSD_PROC_EXIT		1	/* process exit */
#define BSD_PROC_FRCEXIT 	2	/* Kernel force termination */

/* The Kernel Debug Sub Classes for DBG_TRACE */
#define DBG_TRACE_DATA      0
#define DBG_TRACE_STRING    1
#define	DBG_TRACE_INFO	    2

/* The Kernel Debug Sub Classes for DBG_CORESTORAGE */
#define DBG_CS_IO	0

/* Sub-class codes for CoreGraphics (DBG_CG) are defined in its component. */

/* The Kernel Debug Sub Classes for DBG_MISC */
#define DBG_EVENT	0x10
#define	DBG_BUFFER	0x20

/* The Kernel Debug Sub Classes for DBG_DYLD */
#define DBG_DYLD_STRING   5

/* The Kernel Debug modifiers for the DBG_DKRW sub class */
#define DKIO_DONE 	0x01
#define DKIO_READ	0x02
#define DKIO_ASYNC	0x04
#define DKIO_META	0x08
#define DKIO_PAGING	0x10
#define DKIO_THROTTLE	0x20
#define DKIO_PASSIVE	0x40
#define DKIO_NOCACHE	0x80

/* Kernel Debug Sub Classes for Applications (DBG_APPS) */
#define DBG_APP_LOGINWINDOW     0x03
#define DBG_APP_SAMBA           0x80


/**********************************************************************/

#define KDBG_CODE(Class, SubClass, code) (((Class & 0xff) << 24) | ((SubClass & 0xff) << 16) | ((code & 0x3fff)  << 2))

#define KDBG_MIGCODE(msgid) ((DBG_MIG << 24) | (((msgid) & 0x3fffff)  << 2))

#define MACHDBG_CODE(SubClass, code) KDBG_CODE(DBG_MACH, SubClass, code)
#define NETDBG_CODE(SubClass, code) KDBG_CODE(DBG_NETWORK, SubClass, code)
#define FSDBG_CODE(SubClass, code) KDBG_CODE(DBG_FSYSTEM, SubClass, code)
#define BSDDBG_CODE(SubClass, code) KDBG_CODE(DBG_BSD, SubClass, code)
#define IOKDBG_CODE(SubClass, code) KDBG_CODE(DBG_IOKIT, SubClass, code)
#define DRVDBG_CODE(SubClass, code) KDBG_CODE(DBG_DRIVERS, SubClass, code)
#define TRACEDBG_CODE(SubClass,code) KDBG_CODE(DBG_TRACE, SubClass, code)
#define MISCDBG_CODE(SubClass,code) KDBG_CODE(DBG_MISC, SubClass, code)
#define DLILDBG_CODE(SubClass,code) KDBG_CODE(DBG_DLIL, SubClass, code)
#define SECURITYDBG_CODE(SubClass,code) KDBG_CODE(DBG_SECURITY, SubClass, code)
#define DYLDDBG_CODE(SubClass,code) KDBG_CODE(DBG_DYLD, SubClass, code)
#define QTDBG_CODE(SubClass,code) KDBG_CODE(DBG_QT, SubClass, code)
#define APPSDBG_CODE(SubClass,code) KDBG_CODE(DBG_APPS, SubClass, code)
#define CPUPM_CODE(code) IOKDBG_CODE(DBG_IOCPUPM, code)

#define KMEM_ALLOC_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 0)
#define KMEM_ALLOC_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 1)
#define KMEM_FREE_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 2)
#define KMEM_FREE_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 3)
#define ZALLOC_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 4)
#define ZALLOC_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 5)
#define ZFREE_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 6)
#define ZFREE_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 7)

#define PMAP_CODE(code) MACHDBG_CODE(DBG_MACH_PMAP, code)


/*   Usage:
 * kernel_debug((KDBG_CODE(DBG_NETWORK, DNET_PROTOCOL, 51) | DBG_FUNC_START),
 *	offset, 0, 0, 0,0)
 *
 * For ex,
 *
 * #include <sys/kdebug.h>
 *
 * #define DBG_NETIPINIT NETDBG_CODE(DBG_NETIP,1)
 *
 *
 * void
 * ip_init()
 * {
 *	register struct protosw *pr;
 *	register int i;
 *
 *	KERNEL_DEBUG(DBG_NETIPINIT | DBG_FUNC_START, 0,0,0,0,0)
 * 	--------
 *	KERNEL_DEBUG(DBG_NETIPINIT, 0,0,0,0,0)
 * 	--------
 *	KERNEL_DEBUG(DBG_NETIPINIT | DBG_FUNC_END, 0,0,0,0,0)
 * }
 *
 
 */

extern unsigned int kdebug_enable;
#define KDEBUG_ENABLE_TRACE   0x1
#define KDEBUG_ENABLE_ENTROPY 0x2
#define KDEBUG_ENABLE_CHUD    0x4
#define KDEBUG_ENABLE_PPT     0x8

/*
 * Infer the supported kernel debug event level from config option.
 * Use (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) as a guard to protect
 * unaudited debug code.
 */
#define KDEBUG_LEVEL_NONE     0
#define KDEBUG_LEVEL_IST      1
#define KDEBUG_LEVEL_STANDARD 2
#define KDEBUG_LEVEL_FULL     3

#if NO_KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_NONE
#elif IST_KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_IST
#elif KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_FULL
#else
#define KDEBUG_LEVEL KDEBUG_LEVEL_STANDARD
#endif

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#ifdef XNU_KERNEL_PRIVATE
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e)				\
do {									\
if (__improbable(kdebug_enable & ~KDEBUG_ENABLE_PPT))					\
kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)

#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e)				\
do {									\
if (__improbable(kdebug_enable & ~KDEBUG_ENABLE_PPT))					\
kernel_debug1(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)
#else	/* XNU_KERNEL_PRIVATE */
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e)				\
do {									\
if (kdebug_enable & ~KDEBUG_ENABLE_PPT)						\
kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)

#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e)				\
do {									\
if (kdebug_enable & ~KDEBUG_ENABLE_PPT)						\
kernel_debug1(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)
#endif /* XNU_KERNEL_PRIVATE */
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e) do { } while(0)
#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e) do { } while(0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */

/*
 * Specify KDEBUG_PPT to indicate that the event belongs to the
 * limited PPT set.
 */
#define KDEBUG_COMMON (KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_ENTROPY|KDEBUG_ENABLE_CHUD|KDEBUG_ENABLE_PPT)
#define KDEBUG_TRACE  (KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_ENTROPY|KDEBUG_ENABLE_CHUD)
#define KDEBUG_PPT    (KDEBUG_ENABLE_PPT)

/*
 * KERNEL_DEBUG_CONSTANT_IST events provide an audited subset of
 * tracepoints for userland system tracing tools.
 */
#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST)
#ifdef XNU_KERNEL_PRIVATE
#define KERNEL_DEBUG_CONSTANT_IST(type,x,a,b,c,d,e)				\
do {									\
if (__improbable(kdebug_enable & type))					\
kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)
#else /* XNU_KERNEL_PRIVATE */
#define KERNEL_DEBUG_CONSTANT_IST(type,x,a,b,c,d,e)				\
do {									\
if (kdebug_enable & type)						\
kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
(uintptr_t)d,(uintptr_t)e);			\
} while(0)
#endif /* XNU_KERNEL_PRIVATE */
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */
#define KERNEL_DEBUG_CONSTANT_IST(type,x,a,b,c,d,e) do { } while(0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */

#if NO_KDEBUG
#define __kdebug_constant_only __unused
#endif

extern void kernel_debug(
                         uint32_t  debugid,
                         uintptr_t arg1,
                         uintptr_t arg2,
                         uintptr_t arg3,
                         uintptr_t arg4,
                         uintptr_t arg5);

extern void kernel_debug1(
                          uint32_t  debugid,
                          uintptr_t arg1,
                          uintptr_t arg2,
                          uintptr_t arg3,
                          uintptr_t arg4,
                          uintptr_t arg5);


#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL)
#ifdef	XNU_KERNEL_PRIVATE
#define KERNEL_DEBUG(x,a,b,c,d,e)					\
do {									\
if (__improbable(kdebug_enable & ~KDEBUG_ENABLE_PPT))			\
kernel_debug((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
(uintptr_t)c, (uintptr_t)d, (uintptr_t)e);		\
} while(0)

#define KERNEL_DEBUG1(x,a,b,c,d,e)					\
do {									\
if (__improbable(kdebug_enable & ~KDEBUG_ENABLE_PPT))			\
kernel_debug1((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
(uintptr_t)c, (uintptr_t)d, (uintptr_t)e);	\
} while(0)

#define __kdebug_only
#else /* !XNU_KERNEL_PRIVATE */
#define KERNEL_DEBUG(x,a,b,c,d,e)					\
do {									\
if (kdebug_enable & ~KDEBUG_ENABLE_PPT)				\
kernel_debug((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
(uintptr_t)c, (uintptr_t)d, (uintptr_t)e);		\
} while(0)

#define KERNEL_DEBUG1(x,a,b,c,d,e)					\
do {									\
if (kdebug_enable & ~KDEBUG_ENABLE_PPT)				\
kernel_debug1((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
(uintptr_t)c, (uintptr_t)d, (uintptr_t)e);	\
} while(0)
#endif /* XNU_KERNEL_PRIVATE */
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */
#define KERNEL_DEBUG(x,a,b,c,d,e) do {} while (0)
#define KERNEL_DEBUG1(x,a,b,c,d,e) do {} while (0)

#define __kdebug_only __unused
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */

#ifdef KERNEL_PRIVATE
#include <mach/boolean.h>
struct proc;
extern void kdbg_trace_data(struct proc *proc, long *arg_pid);

extern void kdbg_trace_string(struct proc *proc, long *arg1, long *arg2, long *arg3, long *arg4);

extern void kdbg_dump_trace_to_file(const char *);
void start_kern_tracing(unsigned int);
struct task;
extern void kdbg_get_task_name(char*, int, struct task *task);
void disable_wrap(uint32_t *old_slowcheck, uint32_t *old_flags);
void enable_wrap(uint32_t old_slowcheck, boolean_t lostevents);
void release_storage_unit(int cpu,  uint32_t storage_unit);
int allocate_storage_unit(int cpu);

void trace_handler_map_ctrl_page(uintptr_t addr, unsigned long ctrl_page_size, unsigned long storage_size, unsigned long kds_ptr_size);
void trace_handler_map_bufinfo(uintptr_t addr, unsigned long size);
void trace_handler_unmap_bufinfo(void);
void trace_handler_map_buffer(int index, uintptr_t addr, unsigned long size);
void trace_handler_unmap_buffer(int index);
void trace_set_timebases(uint64_t tsc, uint64_t ns);


#endif  /* KERNEL_PRIVATE */


#endif /* __APPLE_API_UNSTABLE */
__END_DECLS


#ifdef	PRIVATE
#ifdef __APPLE_API_PRIVATE
/*
 * private kernel_debug definitions
 */

typedef struct {
	uint64_t	timestamp;
	uintptr_t	arg1;
	uintptr_t	arg2;
	uintptr_t	arg3;
	uintptr_t	arg4;
	uintptr_t	arg5;       /* will hold current thread */
	uint32_t	debugid;
#if defined(__LP64__)
	uint32_t	cpuid;
	uintptr_t	unused;
#endif
} kd_buf;

#if !defined(__LP64__)
#define KDBG_TIMESTAMP_MASK		0x00ffffffffffffffULL
#define KDBG_CPU_MASK			0xff00000000000000ULL
#define KDBG_CPU_SHIFT			56
static inline void
kdbg_set_cpu(kd_buf *kp, int cpu)
{
	kp->timestamp = (kp->timestamp & KDBG_TIMESTAMP_MASK) |
    (((uint64_t) cpu) << KDBG_CPU_SHIFT);
}
static inline int
kdbg_get_cpu(kd_buf *kp)
{
	return (int) (((kp)->timestamp & KDBG_CPU_MASK) >> KDBG_CPU_SHIFT);
}
static inline void
kdbg_set_timestamp(kd_buf *kp, uint64_t thetime)
{
	kp->timestamp = thetime & KDBG_TIMESTAMP_MASK;
}
static inline uint64_t
kdbg_get_timestamp(kd_buf *kp)
{
	return kp->timestamp & KDBG_TIMESTAMP_MASK;
}
static inline void
kdbg_set_timestamp_and_cpu(kd_buf *kp, uint64_t thetime, int cpu)
{
	kp->timestamp = (thetime & KDBG_TIMESTAMP_MASK) |
    (((uint64_t) cpu) << KDBG_CPU_SHIFT);
}
#else
#define KDBG_TIMESTAMP_MASK		0xffffffffffffffffULL
static inline void
kdbg_set_cpu(kd_buf *kp, int cpu)
{
	kp->cpuid = cpu;
}
static inline int
kdbg_get_cpu(kd_buf *kp)
{
	return kp->cpuid;
}
static inline void
kdbg_set_timestamp(kd_buf *kp, uint64_t thetime)
{
	kp->timestamp = thetime;
}
static inline uint64_t
kdbg_get_timestamp(kd_buf *kp)
{
	return kp->timestamp;
}
static inline void
kdbg_set_timestamp_and_cpu(kd_buf *kp, uint64_t thetime, int cpu)
{
	kdbg_set_timestamp(kp, thetime);
	kdbg_set_cpu(kp, cpu);
}
#endif

/* 2^16 bits (8 kilobytes), one for each possible class/subclass combination */
#define KDBG_TYPEFILTER_BITMAP_SIZE	        ( (256 * 256) / 8 )

/* Debug Flags */
#define	KDBG_INIT	0x001
#define	KDBG_NOWRAP	0x002
#define	KDBG_FREERUN	0x004
#define	KDBG_WRAPPED	0x008
#define	KDBG_USERFLAGS	(KDBG_FREERUN|KDBG_NOWRAP|KDBG_INIT)
#define KDBG_PIDCHECK   0x010
#define KDBG_MAPINIT    0x020
#define KDBG_PIDEXCLUDE 0x040
#define KDBG_LOCKINIT	0x080
#define KDBG_LP64	0x100

typedef struct {
	unsigned int	type;
	unsigned int	value1;
	unsigned int	value2;
	unsigned int	value3;
	unsigned int	value4;
	
} kd_regtype;

typedef struct
{
	int		nkdbufs;
	int		nolog;
	int		flags;
	int		nkdthreads;
	int		bufid;
} kbufinfo_t;

typedef struct {
	uintptr_t	thread;
	int		valid;
	char		command[20];
} kd_threadmap;


typedef struct {
	int             version_no;
	int             thread_count;
	uint64_t        TOD_secs;
	uint32_t        TOD_usecs;
} RAW_header;

#define RAW_VERSION0	0x55aa0000
#define RAW_VERSION1	0x55aa0101


#define	KDBG_CLASSTYPE		0x10000
#define	KDBG_SUBCLSTYPE		0x20000
#define	KDBG_RANGETYPE		0x40000
#define	KDBG_TYPENONE		0x80000
#define KDBG_CKTYPES		0xF0000

#define	KDBG_RANGECHECK	0x100000
#define	KDBG_VALCHECK	0x200000        /* Check up to 4 individual values */

#define	KDBG_TYPEFILTER_CHECK	((uint32_t) 0x400000)        /* Check class and subclass against a bitmap */

#define	KDBG_BUFINIT	0x80000000

/* Control operations */
#define	KDBG_EFLAGS	1
#define	KDBG_DFLAGS	2
#define KDBG_ENABLE	3
#define KDBG_SETNUMBUF	4
#define KDBG_GETNUMBUF	5
#define KDBG_SETUP	6
#define KDBG_REMOVE	7
#define	KDBG_SETREGCODE	8
#define	KDBG_GETREGCODE	9
#define	KDBG_READTRACE	10
#define KDBG_PIDTR      11
#define KDBG_THRMAP     12
#define KDBG_PIDEX      14
#define KDBG_SETRTCDEC  15
#define KDBG_KDGETENTROPY 16

/* Minimum value allowed when setting decrementer ticks */
#define KDBG_MINRTCDEC  2500

#endif /* __APPLE_API_PRIVATE */
#endif	/* PRIVATE */

#endif /* !BSD_SYS_KDEBUG_H */

#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/usb/USB.h>

#include "USBTracepoints.h"
	
#ifndef KERNEL_PRIVATE
#define KERNEL_PRIVATE
#include <sys/kdebug.h>
#undef KERNEL_PRIVATE
#else
#include <sys/kdebug.h>
#endif // KERNEL_PRIVATE

#include <AvailabilityMacros.h>


#include "USBTracepoints.h"
#include "IOUSBFamilyInfoPlist.pch"

#define DEBUG 			0

#ifndef USBTRACE_VERSION
	#define	USBTRACE_VERSION "100.4.0"
#endif

#define log(a,b,c,d,x,...)			if ( PrintHeader(a,b,c,d) ) { if (x){fprintf(stdout,x, ##__VA_ARGS__);} fprintf(stdout, "\n"); }
#define logs(a,b,c,x...)			log(a,b,c,0,x...)
#define	vlog(x...)					if ( gVerbose ) { fprintf(stdout,x); }
#define	elog(x...)					fprintf(stderr, x)

//—————————————————————————————————————————————————————————————————————————————
//	Constants
//—————————————————————————————————————————————————————————————————————————————

#define kTraceBufferSampleSize			1000000
#define kMicrosecondsPerSecond			1000000
#define kMicrosecondsPerMillisecond		1000
#define kPrintMaskAllTracepoints		0x80000000
#define kTimeStringSize					44
#define kTimeStampKernel				0x1
#define kTimeStampLocalTime				0x2
#define kPrintStartToken				"->"
#define kPrintEndToken					"<-"
#define kPrintMedialToken				"  "
#define	kFilePathMaxSize				256
#define	kMicrosecondsPerCollectionDelay	20
#define gTimeStampDivisorString			"TimeStampDivisor="
#define kInvalid						0xdeadbeef
#define kDivisorEntry					0xfeedface
#define kKernelTraceCodes				"/usr/local/share/misc/trace.codes"

//—————————————————————————————————————————————————————————————————————————————
//	Types
//—————————————————————————————————————————————————————————————————————————————

typedef struct {
	uint64_t	timestamp;
	uintptr_t	thread;       /* will hold current thread */
	uint32_t	debugid;
	uint32_t	cpuid;
} trace_info;

// Constants that define the different power states
enum
{
    kUSBOff				= 0,				// controller is reset, nothing is attached
    kUSBStateRestart	= 1,				// same as OFF
    kUSBSleep			= 2,				// controller is suspended, preparing to lose main power
    kUSBLowPower		= 3,				// controller is suspended, power remains on
    kUSBOn				= 4,				// up and running
    kUSBBusPowerStates	= 5
};

#define	kIOUSBMessageMuxFromEHCIToXHCI				iokit_usb_msg(0xe1)		// 0xe00040e1  Message from the EHCI HC for ports mux transition from EHCI to XHCI
#define	kIOUSBMessageMuxFromXHCIToEHCI				iokit_usb_msg(0xe2)		// 0xe00040e2  Message from the EHCI HC for ports mux transition from XHCI to EHCI
#define kIOUSBMessageHubPortDeviceDisconnected      iokit_usb_msg(0x1b)		// 0xe000401b  Message sent by a built-in hub when a device was disconnected

enum
{
    kUSBBusStateReset				= 0,				// bus is in RESET
    kUSBBusStateSuspended			= 1,				// bus is in SUSPEND mode
    kUSBBusStateRunning				= 2					// bus is operational
};

//—————————————————————————————————————————————————————————————————————————————
//	OHCI Defines
//—————————————————————————————————————————————————————————————————————————————

enum
{
	kOHCIBit0					= (1 << 0),
	kOHCIBit1					= (1 << 1),
	kOHCIBit2					= (1 << 2),
	kOHCIBit3					= (1 << 3),
	kOHCIBit4					= (1 << 4),
	kOHCIBit5					= (1 << 5),
	kOHCIBit6					= (1 << 6),
	kOHCIBit7					= (1 << 7),
	kOHCIBit8					= (1 << 8),
	kOHCIBit9					= (1 << 9),
	kOHCIBit10					= (1 << 10),
	kOHCIBit11					= (1 << 11),
	kOHCIBit12					= (1 << 12),
	kOHCIBit13					= (1 << 13),
	kOHCIBit14					= (1 << 14),
	kOHCIBit15					= (1 << 15),
	kOHCIBit16					= (1 << 16),
	kOHCIBit17					= (1 << 17),
	kOHCIBit18					= (1 << 18),
	kOHCIBit19					= (1 << 19),
	kOHCIBit20					= (1 << 20),
	kOHCIBit21					= (1 << 21),
	kOHCIBit22					= (1 << 22),
	kOHCIBit23					= (1 << 23),
	kOHCIBit24					= (1 << 24),
	kOHCIBit25					= (1 << 25),
	kOHCIBit26					= (1 << 26),
	kOHCIBit27					= (1 << 27),
	kOHCIBit28					= (1 << 28),
	kOHCIBit29					= (1 << 29),
	kOHCIBit30					= (1 << 30),
	kOHCIBit31					= (1 << 31)
};

#define OHCIBitRange(start, end)				\
(								\
((((UInt32) 0xFFFFFFFF) << (31 - (end))) >>		\
((31 - (end)) + (start))) <<				\
(start)							\
)

#define OHCIBitRangePhase(start, end)				\
(start)

#define EHCIBitRange(start, end)				\
(								\
((((UInt32) 0xFFFFFFFF) << (31 - (end))) >>		\
((31 - (end)) + (start))) <<				\
(start)							\
)

#define EHCIBitRangePhase(start, end)				\
(start)

enum
{
    kOHCIEDControl_FA			= OHCIBitRange (0,  6),
    kOHCIEDControl_FAPhase		= OHCIBitRangePhase (0, 6),
    kOHCIEDControl_EN			= OHCIBitRange (7, 10),
    kOHCIEDControl_ENPhase		= OHCIBitRangePhase (7, 10),
    kOHCIEDControl_D			= OHCIBitRange (11, 12),
    kOHCIEDControl_DPhase		= OHCIBitRangePhase (11, 12),
    kOHCIEDControl_S			= OHCIBitRange (13, 13),
    kOHCIEDControl_SPhase		= OHCIBitRangePhase (13, 13),
    kOHCIEDControl_K			= kOHCIBit14,
    kOHCIEDControl_F			= OHCIBitRange (15, 15),
    kOHCIEDControl_FPhase		= OHCIBitRangePhase (15, 15),
    kOHCIEDControl_MPS			= OHCIBitRange (16, 26),
    kOHCIEDControl_MPSPhase		= OHCIBitRangePhase (16, 26),
	
    kOHCITailPointer_tailP		= OHCIBitRange (4, 31),
    kOHCITailPointer_tailPPhase		= OHCIBitRangePhase (4, 31),
	
    kOHCIHeadPointer_H			= kOHCIBit0,
    kOHCIHeadPointer_C			= kOHCIBit1,
    kOHCIHeadPointer_headP		= OHCIBitRange (4, 31),
    kOHCIHeadPointer_headPPhase		= OHCIBitRangePhase (4, 31),
	
    kOHCINextEndpointDescriptor_nextED		= OHCIBitRange (4, 31),
    kOHCINextEndpointDescriptor_nextEDPhase	= OHCIBitRangePhase (4, 31),
	
    kOHCIEDDirectionTD			= 0,
    kOHCIEDDirectionOut			= 1,
    kOHCIEDDirectionIn			= 2,
	
    kOHCIEDSpeedFull			= 0,
    kOHCIEDSpeedLow			= 1,
	
    kOHCIEDFormatGeneralTD		= 0,
    kOHCIEDFormatIsochronousTD		= 1
};

typedef  UInt8 OHCIEDFormat;	// really only need 1 bit

// General Transfer Descriptor
enum
{
    kOHCIGTDControl_R			= kOHCIBit18,
    kOHCIGTDControl_DP			= OHCIBitRange (19, 20),
    kOHCIGTDControl_DPPhase		= OHCIBitRangePhase (19, 20),
    kOHCIGTDControl_DI			= OHCIBitRange (21, 23),
    kOHCIGTDControl_DIPhase		= OHCIBitRangePhase (21, 23),
    kOHCIGTDControl_T			= OHCIBitRange (24, 25),
    kOHCIGTDControl_TPhase		= OHCIBitRangePhase (24, 25),
    kOHCIGTDControl_EC			= OHCIBitRange (26, 27),
    kOHCIGTDControl_ECPhase		= OHCIBitRangePhase (26, 27),
    kOHCIGTDControl_CC			= OHCIBitRange (28, 31),
    kOHCIGTDControl_CCPhase		= OHCIBitRangePhase (28, 31),
	
    kOHCIGTDPIDSetup			= 0,
    kOHCIGTDPIDOut			= 1,
    kOHCIGTDPIDIn			= 2,
	
    kOHCIGTDNoInterrupt			= 7,
	
    kOHCIGTDDataToggleCarry		= 0,
    kOHCIGTDDataToggle0			= 2,
    kOHCIGTDDataToggle1			= 3,
	
    kOHCIGTDConditionNoError		= 0,
    kOHCIGTDConditionCRC		= 1,
    kOHCIGTDConditionBitStuffing	= 2,
    kOHCIGTDConditionDataToggleMismatch	= 3,
    kOHCIGTDConditionStall		= 4,
    kOHCIGTDConditionDeviceNotResponding	= 5,
    kOHCIGTDConditionPIDCheckFailure	= 6,
    kOHCIGTDConditionUnexpectedPID	= 7,
    kOHCIGTDConditionDataOverrun	= 8,
    kOHCIGTDConditionDataUnderrun	= 9,
    kOHCIGTDConditionBufferOverrun	= 12,
    kOHCIGTDConditionBufferUnderrun	= 13,
    kOHCIGTDConditionNotAccessed	= 15
};

// Isochronous Transfer Descriptor
enum
{
    kOHCIITDControl_SF			= OHCIBitRange (0,15),
    kOHCIITDControl_SFPhase		= OHCIBitRangePhase(0,15),				
    kOHCIITDControl_DI			= OHCIBitRange (21,23),
    kOHCIITDControl_DIPhase		= OHCIBitRangePhase (21,23),
    kOHCIITDControl_FC			= OHCIBitRange (24,26),
    kOHCIITDControl_FCPhase		= OHCIBitRangePhase (24,26),
    kOHCIITDControl_CC			= OHCIBitRange (28,31),
    kOHCIITDControl_CCPhase		= OHCIBitRangePhase (28,31),
	
    // The Offset/PSW words have two slightly different formats depending on whether they have been accessed
    // by the host controller or not.   They are initialized in Offset format, with 3-bit of condition code (=NOTACCESSED)
    // if the OHCI controller accesses this frame it fills in the 4-bit condition code, and the PSW size field contains
    // the number of bytes transferred IN, or 0 for an OUT transaction.
	
    // PSW format bit field definitions
    kOHCIITDPSW_Size			= OHCIBitRange(0,10),
    kOHCIITDPSW_SizePhase		= OHCIBitRangePhase(0,10),
    kOHCIITDPSW_CC			= OHCIBitRange(12,15),
    kOHCIITDPSW_CCPhase 		= OHCIBitRangePhase(12,15),
	
    // offset format bit field definitions
    kOHCIITDOffset_Size			= OHCIBitRange(0,11),
    kOHCIITDOffset_SizePhase		= OHCIBitRangePhase(0,11),
    kOHCIITDOffset_PC			= OHCIBitRange(12,12),
    kOHCIITDOffset_PCPhase		= OHCIBitRangePhase(12,12),
    kOHCIITDOffset_CC			= OHCIBitRange(13,15),
    kOHCIITDOffset_CCPhase 		= OHCIBitRangePhase(13,15),
    kOHCIITDConditionNoError		= 0,
    kOHCIITDConditionCRC		= 1,
    kOHCIITDConditionBitStuffing	= 2,
    kOHCIITDConditionDataToggleMismatch	= 3,
    kOHCIITDConditionStall		= 4,
    kOHCIITDConditionDeviceNotResponding = 5,
    kOHCIITDConditionPIDCheckFailure	= 6,
    kOHCIITDConditionUnExpectedPID	= 7,
    kOHCIITDConditionDataOverrun	= 8,
    kOHCIITDConditionDataUnderrun	= 9,
    kOHCIITDConditionBufferOverrun	= 12,
    kOHCIITDConditionBufferUnderrun	= 13,
    kOHCIITDOffsetConditionNotAccessed	= 7,	// note that this is the "Offset" variant (3-bit) of this condition code
    kOHCIITDConditionNotAccessedReturn	= 15,
    kOHCIITDConditionNotCrossPage	= 0,
    kOHCIITDConditionCrossPage		= 1
};

enum
{
	// Endpoint Characteristics - see EHCI spec table 3-19
	kEHCIEDFlags_FA			= EHCIBitRange (0,  6),
	kEHCIEDFlags_FAPhase		= EHCIBitRangePhase (0, 6),
	kEHCIEDFlags_EN			= EHCIBitRange (8, 11),
	kEHCIEDFlags_ENPhase		= EHCIBitRangePhase (8, 11),
	kEHCIEDFlags_S			= EHCIBitRange (12, 13),
	kEHCIEDFlags_SPhase		= EHCIBitRangePhase (12, 13),
	kEHCIEDFlags_MPS		= EHCIBitRange (16, 26),
	kEHCIEDFlags_MPSPhase		= EHCIBitRangePhase (16, 26),
	
	kEHCIEDFlags_C			= EHCIBitRange (27, 27),
	kEHCIEDFlags_CPhase		= EHCIBitRangePhase (27, 27),
	kEHCIEDFlags_H			= EHCIBitRange (15, 15),
	kEHCIEDFlags_HPhase		= EHCIBitRangePhase (15, 15),
	kEHCIEDFlags_DTC		= EHCIBitRange (14, 14),
	kEHCIEDFlags_DTCPhase		= EHCIBitRangePhase (14, 14),
	
	// Endpoint capabilities - see EHCI spec table 3-20 
	kEHCIEDSplitFlags_Mult		= EHCIBitRange (30, 31),
	kEHCIEDSplitFlags_MultPhase	= EHCIBitRangePhase (30, 31),
	kEHCIEDSplitFlags_Port		= EHCIBitRange (23, 29),
	kEHCIEDSplitFlags_PortPhase	= EHCIBitRangePhase (23, 29),
	kEHCIEDSplitFlags_HubAddr	= EHCIBitRange (16, 22),
	kEHCIEDSplitFlags_HubAddrPhase	= EHCIBitRangePhase (16, 22),
	kEHCIEDSplitFlags_CMask		= EHCIBitRange (8, 15),
	kEHCIEDSplitFlags_CMaskPhase	= EHCIBitRangePhase (8, 15),
	kEHCIEDSplitFlags_SMask		= EHCIBitRange (0, 7),
	kEHCIEDSplitFlags_SMaskPhase	= EHCIBitRangePhase (0, 7),
	
	kEHCIUniqueNumNoDirMask				= kEHCIEDFlags_EN | kEHCIEDFlags_FA,
	
	
	kEHCIEDDirectionTD			= 2,
	//	kEHCIEDDirectionOut			= 1,
	//	kEHCIEDDirectionIn			= 2,
	
	//	kEHCIGTDPIDOut				= 0,
	//	kEHCIGTDPIDIn				= 1,
	//	kEHCIGTDPIDSetup			= 2,
	
	kEHCIEDFormatGeneralTD		= 0,
	kEHCIEDFormatIsochronousTD	= 1
	
};

enum{
	kEHCITyp_iTD 				= 0,
	kEHCITyp_QH 				= 1,
	kEHCITyp_siTD 				= 2,
	kEHCIEDNextED_Typ			= EHCIBitRange (1,  2),
	kEHCIEDNextED_TypPhase		= EHCIBitRangePhase (1,  2),
	kEHCIEDTDPtrMask			= EHCIBitRange (5,  31)
	
};

enum
{
	kXHCIBit0					= (1 << 0),
	kXHCIBit1					= (1 << 1),
	kXHCIBit2					= (1 << 2),
	kXHCIBit3					= (1 << 3),
	kXHCIBit4					= (1 << 4),
	kXHCIBit5					= (1 << 5),
	kXHCIBit6					= (1 << 6),
	kXHCIBit7					= (1 << 7),
	kXHCIBit8					= (1 << 8),
	kXHCIBit9					= (1 << 9),
	kXHCIBit10					= (1 << 10),
	kXHCIBit11					= (1 << 11),
	kXHCIBit12					= (1 << 12),
	kXHCIBit13					= (1 << 13),
	kXHCIBit14					= (1 << 14),
	kXHCIBit15					= (1 << 15),
	kXHCIBit16					= (1 << 16),
	kXHCIBit17					= (1 << 17),
	kXHCIBit18					= (1 << 18),
	kXHCIBit19					= (1 << 19),
	kXHCIBit20					= (1 << 20),
	kXHCIBit21					= (1 << 21),
	kXHCIBit22					= (1 << 22),
	kXHCIBit23					= (1 << 23),
	kXHCIBit24					= (1 << 24),
	kXHCIBit25					= (1 << 25),
	kXHCIBit26					= (1 << 26),
	kXHCIBit27					= (1 << 27),
	kXHCIBit28					= (1 << 28),
	kXHCIBit29					= (1 << 29),
	kXHCIBit30					= (1 << 30),
	kXHCIBit31					= (1 << 31)
};


#define XHCIBitRange(start, end)				\
(								\
((((UInt32) 0xFFFFFFFF) << (31 - (end))) >>		\
((31 - (end)) + (start))) <<				\
(start)							\
)

#define XHCIBitRangePhase(start, end)				\
(start)


enum 
{
	kXHCITRB_Normal = 1,
	kXHCITRB_Setup,
	kXHCITRB_Data,
	kXHCITRB_Status,
	kXHCITRB_Isoc,
	kXHCITRB_Link,
	kXHCITRB_EventData,
	kXHCITRB_TrNoOp,
	
	kXHCITRB_EnableSlot = 9,
	kXHCITRB_DisableSlot = 10,
	kXHCITRB_AddressDevice = 11,
	kXHCITRB_ConfigureEndpoint = 12,
	kXHCITRB_EvaluateContext = 13,
	kXHCITRB_ResetEndpoint = 14,
	kXHCITRB_StopEndpoint = 15,
	kXHCITRB_SetTRDqPtr = 16,
	kXHCITRB_ResetDevice = 17,
    
    kXHCITRB_GetPortBandwidth = 21,
    kXHCITRB_ForceHeaderCommand = 22,
	kXHCITRB_CMDNoOp = 23,
    
	kXHCITRB_CMDNEC = 49,   // NEC vendor specific command to get firmware version
	
	kXHCITRB_TE = 32,
	kXHCITRB_CCE = 33,
	kXHCITRB_PSCE = 34,
	kXHCITRB_DevNot = 38,   // Device notification event.
	kXHCITRB_MFWE = 39,
    kXHCITRB_NECCCE = 48,
	
    // TRT- Transfer type in a Control request TRB.
    kXHCI_TRT_NoData = 0,
    kXHCI_TRT_OutData = 2,
    kXHCI_TRT_InData = 3,
    
    
	kXHCIFrameNumberIncrement = kXHCIBit11,
    
    // Note XHCI spec sec 4.11.2.5 says an Isoc transaction shouldn't be more than 895ms in future.
    kXHCIFutureIsocLimit = 895,
	
	kXHCITRB_C = kXHCIBit0,
	kXHCITRB_DCS = kXHCIBit0,
	kXHCITRB_TC = kXHCIBit1,
	kXHCITRB_ENT = kXHCIBit1,
	kXHCITRB_ISP = kXHCIBit2,
	kXHCITRB_ED = kXHCIBit2,
	kXHCITRB_CH = kXHCIBit4,
	kXHCITRB_IOC = kXHCIBit5,
	kXHCITRB_IDT = kXHCIBit6,
	kXHCITRB_BSR = kXHCIBit9,
	kXHCITRB_BEI = kXHCIBit9,
	kXHCITRB_TSP = kXHCIBit9,
	kXHCITRB_DIR = kXHCIBit16,
    kXHCITRB_SP  = kXHCIBit23,
	
	kXHCITRB_Normal_Len_Mask = XHCIBitRange(0, 16),
	kXHCITRB_TDSize_Mask = XHCIBitRange(17, 21),
	kXHCITRB_TDSize_Shift = XHCIBitRangePhase(17, 21),
	kXHCITRB_InterrupterTarget_Mask = XHCIBitRange(22, 31),
	kXHCITRB_InterrupterTarget_Shift = XHCIBitRangePhase(22, 31),
	kXHCITRB_TBC_Mask = XHCIBitRange(7,8),
	kXHCITRB_TBC_Shift = XHCIBitRangePhase(7,8),
	kXHCITRB_TLBPC_Mask = XHCIBitRange(16, 19),
	kXHCITRB_TLBPC_Shift = XHCIBitRangePhase(16, 19),
	kXHCITRB_Type_Mask = XHCIBitRange(10, 15),
	kXHCITRB_Type_Shift = XHCIBitRangePhase(10, 15),
	kXHCITRB_TRT_Mask = XHCIBitRange(16, 17),
	kXHCITRB_TRT_Shift = XHCIBitRangePhase(16, 17),
	kXHCITRB_SlotID_Mask = XHCIBitRange(24, 31),
	kXHCITRB_SlotID_Shift = XHCIBitRangePhase(24, 31),
	kXHCITRB_TR_Len_Mask = XHCIBitRange(0, 23),
	kXHCITRB_CC_Mask = XHCIBitRange(24, 31),
	kXHCITRB_CC_Shift = XHCIBitRangePhase(24, 31),
	kXHCITRB_Ep_Mask = XHCIBitRange(16, 20),
	kXHCITRB_Ep_Shift = XHCIBitRangePhase(16, 20),
	kXHCITRB_Stream_Mask = XHCIBitRange(16, 31),
	kXHCITRB_Stream_Shift = XHCIBitRangePhase(16, 31),
	kXHCITRB_Port_Mask = XHCIBitRange(24, 31),
	kXHCITRB_Port_Shift = XHCIBitRangePhase(24, 31),
	
    kXHCIStrCtx_SCT_Mask = XHCIBitRange(1,3),
	kXHCIStrCtx_SCT_Shift = XHCIBitRangePhase(1,3),

    // Section 6.4.5 TRB Completion Codes
	kXHCITRB_CC_Invalid = 0,
	kXHCITRB_CC_Success = 1,
	kXHCITRB_CC_Data_Buffer = 2,
	kXHCITRB_CC_Babble_Detected = 3,
	kXHCITRB_CC_XActErr = 4,
	kXHCITRB_CC_TRBErr = 5,
	kXHCITRB_CC_STALL = 6,
	kXHCITRB_CC_ResourceErr = 7,
	kXHCITRB_CC_Bandwidth = 8,
	kXHCITRB_CC_NoSlots = 9,
	kXHCITRB_CC_Invalid_Stream_Type = 10,
	kXHCITRB_CC_Slot_Not_Enabled = 11,
	kXHCITRB_CC_Endpoint_Not_Enabled = 12,
	kXHCITRB_CC_ShortPacket = 13,
	kXHCITRB_CC_RingUnderrun = 14,
	kXHCITRB_CC_RingOverrun = 15,
	kXHCITRB_CC_VF_Event_Ring_Full = 16,
	kXHCITRB_CC_CtxParamErr = 17,
	kXHCITRB_CC_Bandwidth_Overrun = 18,
	kXHCITRB_CC_CtxStateErr = 19,
	kXHCITRB_CC_No_Ping_Response = 20,
	kXHCITRB_CC_Event_Ring_Full = 21,
	kXHCITRB_CC_Incompatible_Device = 22,
	kXHCITRB_CC_Missed_Service = 23,
	kXHCITRB_CC_CMDRingStopped = 24,
	kXHCITRB_CC_Command_Aborted = 25,
	kXHCITRB_CC_Stopped = 26,
	kXHCITRB_CC_Stopped_Length_Invalid = 27,
	kXHCITRB_CC_Max_Exit_Latency_Too_Large = 29,
	kXHCITRB_CC_Isoch_Buffer_Overrun = 31,
	kXHCITRB_CC_Event_Lost = 32,
	kXHCITRB_CC_Undefined = 33,
	kXHCITRB_CC_Invalid_Stream_ID = 34,
	kXHCITRB_CC_Secondary_Bandwidth = 35,
	kXHCITRB_CC_Split_Transaction = 36,
    
    // Intel specifc errors
	kXHCITRB_CC_CNTX_ENTRIES_GTR_MAXEP = 192,
	kXHCITRB_CC_FORCE_HDR_USB2_NO_SUPPORT = 193,
	kXHCITRB_CC_UNDEFINED_BEHAVIOR = 194,
	kXHCITRB_CC_CMPL_VEN_DEF_ERR_195 = 195,
	kXHCITRB_CC_NOSTOP = 196,
	kXHCITRB_CC_HALT_STOP = 197,
	kXHCITRB_CC_DL_ERR = 198,
	kXHCITRB_CC_CMPL_WITH_EMPTY_CONTEXT = 199,
	kXHCITRB_CC_VENDOR_CMD_FAILED = 200,
	
    kXHCITRB_CC_NULLRing = 256, // Fake error to return if you find ring is NULL
    
	kXHCITRB_FrameID_Mask = XHCIBitRange(20, 30),
	kXHCITRB_FrameID_Shift = XHCIBitRangePhase(20, 30),
	kXHCIFrameMask = XHCIBitRange(0,10)	
};



//—————————————————————————————————————————————————————————————————————————————
//	Codes
//—————————————————————————————————————————————————————————————————————————————

#define kTPAllUSB					USB_TRACE ( 0, 0, 0 )

#pragma mark Prototypes
//———————————————————————————————————————————————————————————————————————————
//	Prototypes
//———————————————————————————————————————————————————————————————————————————

static void EnableUSBTracing ( void );
static void EnableTraceBuffer ( int val );
static void SignalHandler ( int signal );
static void GetDivisor ( void );
static void RegisterSignalHandlers ( void );
static void AllocateTraceBuffer ( void );
static void RemoveTraceBuffer ( void );
static void SetTraceBufferSize ( int nbufs );
static void GetTraceBufferInfo ( kbufinfo_t * val );
static void Quit ( const char * s );
static void ResetDebugFlags ( void );
static void InitializeTraceBuffer ( void );
static void Reinitialize ( void );
static void SetInterest ( unsigned int type );
static void ParseArguments ( int argc, const char * argv[] );
static void PrintUsage ( void );

static void CollectTrace ( void );
static void CollectWithAlloc( void );
static void ProcessTracepoint( kd_buf tracepoint );

static void CollectTraceController( kd_buf tracepoint );		
static void CollectTraceControllerUserClient( kd_buf tracepoint );
static void CollectTraceDevice ( kd_buf tracepoint ); //2,
static void CollectTraceDeviceUserClient ( kd_buf tracepoint ); //3,
static void CollectTraceHub ( kd_buf tracepoint ); //4,
static void CollectTraceHubPort ( kd_buf tracepoint ); //5,
static void CollectTraceHSHubUserClient ( kd_buf tracepoint ); //6,
static void CollectTraceHID	( kd_buf tracepoint ); //7,
static void CollectTracePipe ( kd_buf tracepoint ); //8,
static void CollectTraceInterfaceUserClient	( kd_buf tracepoint ); //9,

static void CollectTraceEnumeration( kd_buf tracepoint ); //10
// UIM groupings
static void CollectTraceUHCI ( kd_buf tracepoint ); //11,
static void CollectTraceUHCIUIM	( kd_buf tracepoint ); 
static void CollectTraceUHCIInterrupts ( kd_buf tracepoint ); //13,
static void CollectTraceOHCI ( kd_buf tracepoint ); //14,
static void CollectTraceOHCIInterrupts ( kd_buf tracepoint ); //15,
static void CollectTraceOHCIDumpQs ( kd_buf tracepoint ); //16,
static void CollectTraceEHCI ( kd_buf tracepoint ); //20,
static void CollectTraceEHCIUIM	( kd_buf tracepoint ); //21,
static void CollectTraceEHCIHubInfo	( kd_buf tracepoint ); //22,
static void CollectTraceEHCIInterrupts	( kd_buf tracepoint ); //23,
static void CollectTraceEHCIDumpQs ( kd_buf tracepoint ); //24,
static void CollectTraceXHCI ( kd_buf tracepoint );				//20,
static void CollectTraceXHCIInterrupts	( kd_buf tracepoint ); //23,
static void CollectTraceXHCIRootHubs	( kd_buf tracepoint ); //24,
static void CollectTraceXHCIPrintTRB	( kd_buf tracepoint ); //25,

static void CollectTraceHubPolicyMaker	( kd_buf tracepoint ); //35,
static void CollectTraceCompositeDriver ( kd_buf tracepoint ); //36,
// Actions
static void CollectTraceOutstandingIO ( kd_buf tracepoint ); //42,

// Other drivers
static void CollectTraceAudioDriver ( kd_buf tracepoint ); //50,

static void CollectTraceUnknown ( kd_buf tracepoint );

static void CollectCodeFile ( void );
static void ReadRawFile( const char * filepath );
static void CollectToRawFile ( FILE * file );
static void PrependDivisorEntry ( FILE * file );
static void CollectTraceUnknown ( kd_buf tracepoint );
static char * DecodeID ( uint32_t id, char * string, const int max );
static void CollectTraceBasic ( kd_buf tracepoint );

static char * ConvertTimestamp ( uint64_t timestamp, char * timestring );
static bool PrintHeader ( trace_info info, const char * group, const char * method, uintptr_t theThis );
static void TabIndent ( int numOfTabs );
static void Indent ( int numOfTabs );
static void IndentIn ( int numOfTabs );
static void IndentOut ( int numOfTabs );

const char * DecodeUSBTransferType( uint32_t type );
const char * DecodeUSBPowerState( uint32_t type );
const char * DecodeXHCIMuxTransitionMessage( uint32_t type );
const char * DecodeUSBBusState( uint32_t type );
const char * DecodeTestMode( uint32_t type );
const char * PrintXHCICommandTRBs(trace_info *info, uintptr_t xhci, uint32_t offs0, uint32_t offs4, uint32_t offs8, uint32_t offsC);

const char * GetSpeedName(UInt32 speed);

