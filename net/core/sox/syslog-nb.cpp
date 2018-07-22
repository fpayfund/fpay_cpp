/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog-nb.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>	// for writev
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "syslog-nb.h"

#define ftell(s) INTUSE(_IO_ftell) (s)

static int	LogType = SOCK_DGRAM;	/* type of socket connection */
static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat;		/* status bits, set by openlog() */
static const char *LogTag;		/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
extern char	*__progname;		/* Program name, from crt0. */

static const size_t LOG_MAXLEN = 2048;	/* 2048 should be enough */

static pthread_mutex_t syslog_lock = PTHREAD_MUTEX_INITIALIZER;

static void openlog_internal_nb(const char *, int, int);
static void closelog_internal_nb(void);
static void __syslog_chk_nb(int pri, int flag, const char *fmt, ...);
static void __vsyslog_chk_nb(int pri, int flag, const char *fmt, va_list ap);

static void
cancel_handler (void *ptr)
{
	/*  Free the lock.  */
	if (ptr != NULL)
		free(ptr);
	pthread_mutex_unlock(&syslog_lock);
}

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
syslog_nb(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__vsyslog_chk_nb(pri, -1, fmt, ap);
	va_end(ap);
}

static void
__syslog_chk_nb(int pri, int flag, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__vsyslog_chk_nb(pri, flag, fmt, ap);
	va_end(ap);
}

static void
__vsyslog_chk_nb(int pri, int flag, const char *fmt, va_list ap)
{
	struct tm now_tm;
	time_t now;
	int fd;
//	FILE *f;
	char *buf = 0;
	size_t bufsize = 0;
	size_t prioff, msgoff;
	int len = 0;
	int saved_errno = errno;
	char failbuf[3 * sizeof (pid_t) + sizeof "out of memory []"];

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog_nb(INTERNALLOG, "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if ((LOG_MASK (LOG_PRI (pri)) & LogMask) == 0)
		return;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the message in a memory buffer */
	/* we are not in glibc, that's much convenience, haha */
	buf = (char*)malloc(LOG_MAXLEN);
	if (buf == NULL) {
		pid_t pid = getpid();
		bufsize = snprintf(failbuf, sizeof(failbuf), "out of memory [%d]", pid) + 1;
		buf = failbuf;
		msgoff = 0;
	}
	else {
		/* format should be "<pri>Jan 16 12:13:14 progname[pid]: xxxxxxxxx" */
		bufsize = snprintf(buf, LOG_MAXLEN, "<%d>", pri);
		prioff = bufsize;
		time(&now);
		bufsize += strftime(buf + bufsize, LOG_MAXLEN - bufsize, "%h %e %T ",
							localtime_r(&now, &now_tm));
		msgoff = bufsize;

		if (LogTag == NULL)
			LogTag = __progname;
		if (LogTag == NULL){
			if (LogStat & LOG_PID) {
				bufsize += snprintf(buf + bufsize, LOG_MAXLEN - bufsize, "[%d]",
									(int)getpid());
			}
		}
		else {
			if (LogStat & LOG_PID)
				bufsize += snprintf(buf + bufsize, LOG_MAXLEN - bufsize, "%s[%d]: ",
									LogTag, (int)getpid());
			else
				bufsize += snprintf(buf + bufsize, LOG_MAXLEN - bufsize, "%s: ",
									LogTag);

		}
		//set_error(saved_errno);
		errno = saved_errno;
		// this may fail, for exmaple, the fmt is illegal, 
		// in this case, some char has already printed in
		if ( (len = vsnprintf(buf + bufsize, LOG_MAXLEN - bufsize, fmt, ap)) < 0){
			bufsize += snprintf(buf + bufsize, LOG_MAXLEN - bufsize, "illegal format string:\"%s\"", fmt);
		}
		else {
			bufsize += len;
		}
		if (bufsize > LOG_MAXLEN) {
			// buffer overflow, just trunc it
			bufsize = LOG_MAXLEN - 1; // last char is '\0'
		}
	}
	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = buf + msgoff;
		v->iov_len = bufsize - msgoff;
		/* Append a newline if necessary.  */
		if (buf[bufsize - 1] != '\n') {
			++v;
			v->iov_base = (char *) "\n";
			v->iov_len = 1;
		}

		/* writev is a cancellation point.  */
		pthread_cleanup_push (free, buf == failbuf ? NULL : buf);
		writev(STDERR_FILENO, iov, v - iov + 1);
		pthread_cleanup_pop(0);
	}

	/* Prepare for multiple users.  We have to take care: open and
	   write are cancellation points.  */
	pthread_cleanup_push (cancel_handler, buf == failbuf ? NULL : buf);
	pthread_mutex_lock(&syslog_lock);

	/* Get connected, output the message to the local logger. */
	if (!connected)
		openlog_internal_nb(LogTag, LogStat | LOG_NDELAY, 0);

	/* If we have a SOCK_STREAM connection, also send ASCII NUL as
	   a record terminator.  */
	if (LogType == SOCK_STREAM)
		++bufsize;

	if (!connected || (send(LogFile, buf, bufsize, MSG_NOSIGNAL|MSG_DONTWAIT) < 0 && errno != EAGAIN )) {
	    if (connected){
		/* Try to reopen the syslog connection.  Maybe it went down.  */
			closelog_internal_nb();
			openlog_internal_nb(LogTag, LogStat | LOG_NDELAY, 0);
		}

	    if (!connected || (send(LogFile, buf, bufsize, MSG_NOSIGNAL|MSG_DONTWAIT) < 0 && errno != EAGAIN )) {
			closelog_internal_nb();	/* attempt re-open next time */
			/*
			 * Output the message to the console; don't worry about blocking, 
			 * if console blocks everything will. Make sure the error reported 
			 * is the one from the syslogd failure.
			 */
			if (LogStat & LOG_CONS && (fd = open(_PATH_CONSOLE, O_WRONLY|O_NOCTTY, 0)) >= 0) {
				dprintf (fd, "%s\r\n", buf + msgoff);
				(void)close(fd);
			}
		}
	}

	/* End of critical section.  */
	pthread_cleanup_pop(0);
	pthread_mutex_unlock(&syslog_lock);

	if (buf != failbuf)
		free (buf);
}

void
vsyslog_nb(int pri, const char *fmt, va_list ap)
{
  __vsyslog_chk_nb(pri, -1, fmt, ap);
}

static struct sockaddr_un SyslogAddr;	/* AF_UNIX address of local logger */

static void
openlog_internal_nb(const char *ident, int logstat, int logfac)
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	int retry = 0;
	while (retry < 2) {
		if (LogFile == -1) {
			SyslogAddr.sun_family = AF_UNIX;
			strncpy(SyslogAddr.sun_path, _PATH_LOG, sizeof(SyslogAddr.sun_path));
			if (LogStat & LOG_NDELAY) {
				LogFile = socket(AF_UNIX, LogType, 0);
				if (LogFile == -1)
					return;
				fcntl(LogFile, F_SETFD, FD_CLOEXEC);
			}
		}
		if (LogFile != -1 && !connected)
		{
			int old_errno = errno;
			if (connect(LogFile, (struct sockaddr*)&SyslogAddr, sizeof(SyslogAddr)) == -1)
			{
				int saved_errno = errno;
				int fd = LogFile;
				LogFile = -1;
				close(fd);
				//__set_errno (old_errno);
				errno = old_errno;
				if (saved_errno == EPROTOTYPE)
				{
					/* retry with the other type: */
					LogType = (LogType == SOCK_DGRAM ? SOCK_STREAM : SOCK_DGRAM);
					++retry;
					continue;
				}
			} else
				connected = 1;
		}
		break;
	}
}

void
openlog_nb (const char *ident, int logstat, int logfac)
{
  /* Protect against multiple users and cancellation.  */
  pthread_mutex_lock(&syslog_lock);

  openlog_internal_nb(ident, logstat, logfac);

  pthread_mutex_unlock(&syslog_lock);
}

static void
closelog_internal_nb()
{
  if (!connected)
    return;

  close (LogFile);
  LogFile = -1;
  connected = 0;
}

void
closelog_nb ()
{
  /* Protect against multiple users and cancellation.  */
  pthread_mutex_lock(&syslog_lock);

  closelog_internal_nb();
  LogTag = NULL;
  LogType = SOCK_DGRAM; /* this is the default */

  /* Free the lock.  */
  pthread_mutex_unlock(&syslog_lock);
}

/* setlogmask -- set the log mask level */
int
setlogmask_nb(int pmask)
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
