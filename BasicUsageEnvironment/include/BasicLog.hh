#ifndef _BASIC_USAGE_ENVIRONMENT_HH
#define _BASIC_USAGE_ENVIRONMENT_HH

#include <stdarg.h>

typedef void (*log_handle_fn_t)(const char* fmt, va_list vl);

/** 
 * @brief ������־����ʽ�ص�����
 * 
 * @param fn[in] : �ص�����ָ��
 *
 * @note
 *        ���û������ã���ʹ��Ĭ�ϴ���ʽ������ӡ���ն�
 */
void rtspLogSetCallback(log_handle_fn_t fn);

/** 
 * @brief ��־��ӡ������
 * 
 * @param fmt[in]   : �ɱ����
 *
 */
extern void rtspLog(const char* fmt, ...)__attribute__((format(printf,1,2)));

#endif
