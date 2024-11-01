#ifndef __idtype_t_defined
#define __idtype_t_defined

/* The following values are used by the `waitid' function.  */
typedef enum
{
        /*
         * These names were mostly lifted from Solaris source code and
         * still use Solaris style naming to avoid breaking any
         * OpenSolaris code which has been ported to FreeBSD.  There
         * is no clear FreeBSD counterpart for all of the names, but
         * some have a clear correspondence to FreeBSD entities.
         *
         * The numerical values are kept synchronized with the Solaris
         * values.
         */
        P_PID,                  /* A process identifier. */
        P_PPID,                 /* A parent process identifier. */
        P_PGID,                 /* A process group identifier. */
        P_SID,                  /* A session identifier. */
        P_CID,                  /* A scheduling class identifier. */
        P_UID,                  /* A user identifier. */
        P_GID,                  /* A group identifier. */
        P_ALL,                  /* All processes. */
        P_LWPID,                /* An LWP identifier. */
        P_TASKID,               /* A task identifier. */
        P_PROJID,               /* A project identifier. */
        P_POOLID,               /* A pool identifier. */
        P_JAILID,               /* A zone identifier. */
        P_CTID,                 /* A (process) contract identifier. */
        P_CPUID,                /* CPU identifier. */
        P_PSETID                /* Processor set identifier. */
} idtype_t;                     /* The type of id_t we are using. */

# if defined __USE_MISC
#  define P_ZONEID        P_JAILID
# endif

#endif
