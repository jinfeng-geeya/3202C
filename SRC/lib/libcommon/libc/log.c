#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
#include <sys/syslog.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#if	(SYS_OS_MODULE == ALI_TDS2||SYS_OS_MODULE == NO_OS)
#include <stdarg.h>
#endif

#if	(SYS_OS_MODULE == ALI_TDS2||SYS_OS_MODULE == NO_OS)
#include <osal/osal_timer.h>
#endif
#include <api/libc/string.h>
#include <api/libc/log.h>

#define ERR_COLOR_CNTRL    "\033[31m"  /* turn on with red setting */
#define NOTICE_COLOR_CNTRL "\033[33m"  /* turn on with yellow setting */
#define DEBUG_COLOR_CNTRL  "\033[34m"  /* turn on with blue setting */
#define OFF_COLOR_CNTRL    "\033[0m"   /* turn off all settings */

static amslog_t amslog[LOG_NUM] =
{
	{"APP", 0},
	{"AMS", 0},
	{"EPG", 0},
	{"DB", 0},
	//, add here
	{"NULL", 0},
};

static UINT8 amslog_dest;

void amslog_log(UINT8 level, const char *func, UINT32 line_num,
		amslog_mod_e log_mod, const char *pFmt, ... )
{
	if (amslog[log_mod].log_level < level)
		return;

	va_list		ap;
	char buf[MAX_LOG_LINE_LENGTH] = {0};
	UINT32 len = 0, max_len;
	char *log_level_str = NULL;
	INT32 log_telnet_fd = -1;
	char *color_cntrl = NULL;

#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	max_len = sizeof(buf);
#else
	max_len = sizeof(buf) - sizeof(OFF_COLOR_CNTRL) - 1;
#endif

	va_start(ap, pFmt);

	if (log_mod != (LOG_NUM+1))
	{
		len = snprintf(buf, max_len, "[%s:", amslog[log_mod].mod_name);
	}
	else
	{
		len = snprintf(buf, max_len, "[unknown:");
	}

	if(max_len <= len)
		goto EXIT_CODE;

	/*
	 * Only log the severity level when going to stderr
	 * because syslog already logs the severity level for us.
	 */
#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	if (amslog_dest == LOG_DEST_STDERR)
#endif
	{
		switch(level)
		{
			case LOG_ERR:
				log_level_str = "error";
				color_cntrl = ERR_COLOR_CNTRL;
				break;
			case LOG_NOTICE:
				log_level_str = "notice";
				color_cntrl = NOTICE_COLOR_CNTRL;
				break;
			case LOG_DEBUG:
				log_level_str = "debug";
				color_cntrl = DEBUG_COLOR_CNTRL;
				break;
			default:
				log_level_str = "invalid";
				color_cntrl = OFF_COLOR_CNTRL;
				break;
		}
		len += snprintf(&(buf[len]), max_len - len, "%s:", log_level_str);
	}

	if (max_len <= len)
		goto EXIT_CODE;

#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	{
		struct timespec stime;

		clock_gettime(CLOCK_REALTIME, &stime);
		len += snprintf(&(buf[len]), max_len - len, "%u.%03u:",
					stime.tv_sec%1000, stime.tv_nsec/1000000);
	}

	if (max_len <= len)
		goto EXIT_CODE;
#else
	{
		UINT32 curr_time;

		curr_time = osal_get_tick();
		len += snprintf(&(buf[len]), max_len - len, "%d:", curr_time);
	}
#endif

	len += snprintf(&(buf[len]), max_len - len, "%s:%u]", func, line_num);

	/* add ANSI control code to print message with color */
	len += snprintf(&(buf[len]), max_len - len, "%s", color_cntrl);

	if (max_len <= len)
		goto EXIT_CODE;

	max_len -= len;
	len += vsnprintf(&buf[len], max_len, pFmt, ap);

#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	if (amslog_dest == LOG_DEST_STDERR)
	{
		fprintf(stderr, "%s%s", buf, OFF_COLOR_CNTRL);
	}
	else if (amslog_dest == LOG_DEST_TELNET)
	{
#if 0
		/* Fedora Desktop Linux */
		log_telnet_fd = open("/dev/pts/1", O_RDWR);
#else
		/* CPE use ptyp0 as the first pesudo terminal */
		log_telnet_fd = open("/dev/ttyp0", O_RDWR);
#endif
		if(log_telnet_fd != -1)
		{
			write(log_telnet_fd, buf, strlen(buf));
			write(log_telnet_fd, "\n", strlen("\n"));
			close(log_telnet_fd);
		}
	}
	else
	{
		syslog(level, buf);
	}
#else /* for TDS printf */
	{
		snprintf(&(buf[len]), sizeof(buf) - len, "%s", OFF_COLOR_CNTRL);
		vprintf(buf, ap);
	}
#endif

EXIT_CODE:
	va_end(ap);
}

void amslog_init(void)
{
	UINT8 i;

	for(i = 0; i < (LOG_NUM + 1); ++i)
	{
		amslog[i].log_level = DEFAULT_LOG_LEVEL;
	}

	amslog_dest = DEFAULT_LOG_DESTINATION;

#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	openlog(NULL, 0, LOG_DAEMON);
#endif
}

RET_CODE amslog_reg(INT32 mod_index, amslog_t *plog)
{    
	if (mod_index >= 0 && mod_index < (LOG_NUM+1))
	{
		if (STRLEN(plog->mod_name) < MAX_LOG_NAME_LENGTH)
			STRCPY(amslog[mod_index].mod_name, plog->mod_name);
		else
		{
			//error
			//#@$#@%, wait string.c to add STRNCPY function!!!!!
			//STRNCPY(amslog[mod_index].mod_name, plog->mod_name, MAX_LOG_NAME_LENGTH-1);
			//amslog[mod_index].mod_name[MAX_LOG_NAME_LENGTH-1] = '\0';
		}

        amslog[mod_index].log_level = plog->log_level;
        
		return SUCCESS;
	}
	else
	{
		//error!
		return ERR_PARA;
	}
}

void amslog_cleanup(void)
{
#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	closelog();
#endif
}

void amslog_set_level(amslog_mod_e log_mod, UINT8 log_level)
{
	amslog[log_mod].log_level = log_level;
}

UINT8 amslog_get_level(amslog_mod_e log_mod)
{
	return amslog[log_mod].log_level;
}

void amslog_set_dest(amslog_dest_e dest)
{
	amslog_dest = dest;
}

amslog_dest_e amslog_get_dest(void)
{
	return amslog_dest;
}

///////////////////////////////////////////////////////////////////////////////////
//Sample Code
void log_sample()
{
    /* Init log */
	amslog_init();

    /* Set log print level */
	amslog_set_level(LOG_APP, LOG_DEBUG);
	amslog_set_level(LOG_DB, LOG_ERR);

	amslog_debug(LOG_APP, "App Init ok.\n");
	amslog_error(LOG_APP, "get DSC Dev failed!\n");
	amslog_notice(LOG_APP, "this is a log test\n");
	amslog_debug(LOG_APP, "APP init finished...\n");

	amslog_cleanup();
}

