#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

volatile UINT32 s_debug_level = DEBUG_LEVEL_DFAULT;

UINT8 Debug_GetLevel(void)
{
	return s_debug_level;
}

void Debug_SetLevel(UINT8 new_level)
{
	s_debug_level = new_level;
}

void pdebughex(UINT8 *src, UINT16 len)
{
	INT32 i;

	if(s_debug_level >= DEBUG_LEVEL_DEBUG)
	{
		for(i = 0; i < len; i++)
		{
			if(i != (len-1))
			{
				printf("%02X,", src[i]);
			}
			else
			{
				printf("%02X.\r\n", src[i]);
			}
		}
	}
}

void perrhex(UINT8 *src, UINT16 len)
{
	INT32 i;

	if(s_debug_level >= DEBUG_LEVEL_ERROR)
	{
		for(i = 0; i < len; i++)
		{
			if(i != (len-1))
			{
				printf("%02X,", src[i]);
			}
			else
			{
				printf("%02X.\r\n", src[i]);
			}
		}
	}
}

void phex(UINT8 *src, UINT16 len)
{
	INT32 i;

		for(i = 0; i < len; i++)
		{
			if(i != (len-1))
			{
				printf("%02X,", src[i]);
			}
			else
			{
				printf("%02X\r\n", src[i]);
			}
		}
}

#if 0
void pdebug(const char *format, ...)
{
		va_list args;
		
		printf("dbg: ");	
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
}

void perr(const char *format, ...)
{
		va_list args;
		
		printf("err: ");
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
}

void pinfo(const char *format, ...)
{
		va_list args;

		printf("info: ");	
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
}
#else

void pdebug(const char *format, ...)
{
	if(s_debug_level >= DEBUG_LEVEL_DEBUG)
	{
		va_list args;
		
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
	}
}

void perr(const char *format, ...)
{
	if(s_debug_level >= DEBUG_LEVEL_ERROR)
	{
		va_list args;
		
		printf("err: ");
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
	}
}

void pinfo(const char *format, ...)
{
	if(s_debug_level >= DEBUG_LEVEL_INFO)
	{
		va_list args;
		
		va_start(args,format);
		vprintf(format,args);
		va_end(args);		
	}
}
#endif

void pprint(const char *format, ...)
{
	va_list args;
	
	va_start(args,format);
	vprintf(format,args);
	va_end(args);
}

