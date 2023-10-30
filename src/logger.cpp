#include "logger.h"

#include "entity_manager.h"

#include <stdarg.h>

#define FORMAT_MESSAGE_STARTWITH "[%s]: "
#define FORMAT_MESSAGE FORMAT_MESSAGE_STARTWITH "%s\n"

#define FORMAT_WARNING_STARTWITH "[%s] WARNING: "
#define FORMAT_WARNING FORMAT_WARNING_STARTWITH "%s\n"

#define FORMAT_ERROR_STARTWITH "[%s] ERROR "
#define FORMAT_ERROR FORMAT_ERROR_STARTWITH "%s\n"

#define FORMAT_DEV_MESSAGE_STARTWITH "[%s] DEV L%d: "
#define FORMAT_DEV_MESSAGE FORMAT_DEV_MESSAGE_STARTWITH "%s\n"

#define FORMAT_DEV_WARNING_STARTWITH "[%s] DEV WARNING L%d: "
#define FORMAT_DEV_WARNING FORMAT_DEV_WARNING_STARTWITH "%s\n"

Logger::Logger(const char *pszName, RegisterTagsFunc pfnRegisterTagsFunc, int iFlags, LoggingVerbosity_t eVerbosity, const Color &aDefault)
{
	this->m_nChannelID = LoggingSystem_RegisterLoggingChannel(pszName, pfnRegisterTagsFunc, iFlags, eVerbosity, aDefault);
}

void Logger::Detailed(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_DETAILED, pszContent);
}

void Logger::Detailed(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_DETAILED, aColor, pszContent);
}

void Logger::DetailedFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Detailed((const char *)this->m_sFormatBuffer);
}

void Logger::DetailedFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Detailed(aColor, (const char *)this->m_sFormatBuffer);
}

void Logger::Message(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_MESSAGE, pszContent);
}

void Logger::Message(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_MESSAGE, aColor, pszContent);
}

void Logger::MessageFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Message((const char *)this->m_sFormatBuffer);
}

void Logger::MessageFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Message(aColor, (const char *)this->m_sFormatBuffer);
}

void Logger::Warning(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_WARNING, pszContent);
}

void Logger::Warning(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_WARNING, aColor, pszContent);
}

void Logger::WarningFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Warning((const char *)this->m_sFormatBuffer);
}

void Logger::WarningFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Warning(aColor, (const char *)this->m_sFormatBuffer);
}

void Logger::ThrowAssert(const char *pszFilename, int iLine, const char *pszContent)
{
	LoggingSystem_Log(this->m_nChannelID, LS_ASSERT, "%s (%d) : %s", pszFilename, iLine, pszContent);
}

void Logger::ThrowAssert(const char *pszFilename, int iLine, Color aColor, const char *pszContent)
{
	LoggingSystem_Log(this->m_nChannelID, LS_ASSERT, aColor, "%s (%d) : %s", pszFilename, iLine, pszContent);
}

void Logger::ThrowAssertFormat(const char *pszFilename, int iLine, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->ThrowAssert(pszFilename, iLine, (const char *)this->m_sFormatBuffer);
}

void Logger::ThrowAssertFormat(const char *pszFilename, int iLine, Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->ThrowAssert(pszFilename, iLine, aColor, (const char *)this->m_sFormatBuffer);
}

void Logger::Error(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_ERROR, pszContent);
}

void Logger::Error(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_ERROR, aColor, pszContent);
}

void Logger::ErrorFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Error((const char *)this->m_sFormatBuffer);
}

void Logger::ErrorFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Error(aColor, (const char *)this->m_sFormatBuffer);
}

void Logger::DoTests()
{ 
	this->DetailedFormat("LS_DETAILED = %d\n", LS_DETAILED);
	this->MessageFormat("LS_MESSAGE = %d\n", LS_MESSAGE);
	this->WarningFormat("LS_WARNING = %d\n", LS_WARNING);
	this->ThrowAssertFormat(__FILE__, __LINE__, "LS_ASSERT = %d\n", LS_ASSERT);
	this->ErrorFormat("LS_ERROR = %d\n", LS_ERROR);
}

Logger::Scope::Scope(const char *pszStartWith)
{
	this->m_aStartWith = pszStartWith;
	this->m_aEnd = "\n";
}

Logger::Scope::Scope(const char *pszStartWith, const char *pszEnd)
{
	this->m_aStartWith = pszStartWith;
	this->m_aEnd = pszEnd;
}

size_t Logger::Scope::Count()
{
	return this->m_vec.size();
}

size_t Logger::Scope::Push(const char *pszContent)
{
	Message aMsg;

	size_t nStoredLength = aMsg.SetWithCopy(pszContent);

	this->m_vec.push_back(aMsg);

	return nStoredLength;
}

size_t Logger::Scope::PushFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	Message aMsg;

	size_t nStoredLength = aMsg.SetWithCopy((const char *)this->m_sFormatBuffer);

	this->m_vec.push_back(aMsg);

	return nStoredLength;
}

size_t Logger::Scope::Send(SendFunc funcOn)
{
	std::string sResultContent;

	size_t nSize = this->m_vec.size();
	
	for(size_t n = 0; n < nSize; n++)
	{
		sResultContent += this->m_aStartWith + this->m_vec[n].Get() + this->m_aEnd;
	}

	funcOn(sResultContent.c_str());

	return nSize;
}

const std::string &Logger::Scope::Message::Get() const
{
	return this->m_sContent;
}

size_t Logger::Scope::Message::SetWithCopy(const char *pszContent)
{
	this->m_sContent = pszContent;

	return this->m_sContent.size();
}

Logger::Scope Logger::CreateDetailsScope()
{
	return {};
}

Logger::Scope Logger::CreateMessagesScope()
{
	return {};
}

Logger::Scope Logger::CreateWarningsScope()
{
	return {};
}

Logger::Scope Logger::CreateAssertScope()
{
	return {};
}

Logger::Scope Logger::CreateErrorsScope()
{
	return {};
}
