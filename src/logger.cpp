#include "logger.h"

#include "entity_manager.h"

#include <stdarg.h>

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

Logger::Scope::Scope(const Color &rgba, const char *pszStartWith)
{
	this->m_aColor = rgba;

	this->m_aStartWith = pszStartWith;
	this->m_aEnd = "\n";
}

Logger::Scope::Scope(const Color &rgba, const char *pszStartWith, const char *pszEnd)
{
	this->m_aColor = rgba;

	this->m_aStartWith = pszStartWith;
	this->m_aEnd = pszEnd;
}

const Color &Logger::Scope::GetColor() const
{
	return this->m_aColor;
}

size_t Logger::Scope::Count()
{
	return this->m_vec.size();
}

void Logger::Scope::SetColor(const Color &rgba)
{
	this->m_aColor = rgba;
}

size_t Logger::Scope::Push(const char *pszContent)
{
	Message aMsg(this->m_aColor);

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

	Message aMsg(this->m_aColor);

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

size_t Logger::Scope::SendColor(SendColorFunc funcOn)
{
	std::string sResultContent;

	size_t nSize = this->m_vec.size();

	Color rgbaSave = this->m_aColor;

	for(size_t n = 0; n < nSize; n++)
	{
		const auto &aMsg = this->m_vec[n];

		if(aMsg.GetColor() == rgbaSave)
		{
			sResultContent += this->m_aStartWith + aMsg.Get() + this->m_aEnd;
		}
		else
		{
			funcOn(rgbaSave, sResultContent.c_str());

			sResultContent = this->m_aStartWith + aMsg.Get() + this->m_aEnd;
			rgbaSave = aMsg.GetColor();
		}
	}

	if(sResultContent.size())
	{
		funcOn(rgbaSave, sResultContent.c_str());
	}

	return nSize;
}

Logger::Scope::Message::Message(const Color &rgbaInit)
 :  m_aColor(rgbaInit)
{
}

const Color &Logger::Scope::Message::GetColor() const
{
	return this->m_aColor;
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
#ifdef DEBUG
	return {LOGGER_COLOR_DETAILED, LOGGER_FORMAT_DETAILED_STARTWITH};
#else
	return {LOGGER_COLOR_DETAILED};
#endif
}

Logger::Scope Logger::CreateMessagesScope()
{
#ifdef DEBUG
	return {LOGGER_COLOR_MESSAGE, LOGGER_FORMAT_MESSAGE_STARTWITH};
#else
	return {LOGGER_COLOR_MESSAGE};
#endif
}

Logger::Scope Logger::CreateWarningsScope()
{
#ifdef DEBUG
	return {LOGGER_COLOR_WARNING, LOGGER_FORMAT_WARNING_STARTWITH};
#else
	return {LOGGER_COLOR_WARNING};
#endif
}

Logger::Scope Logger::CreateAssertScope()
{
#ifdef DEBUG
	return {LOGGER_COLOR_ASSERT, LOGGER_FORMAT_ASSERT_STARTWITH};
#else
	return {LOGGER_COLOR_ASSERT};
#endif
}

Logger::Scope Logger::CreateErrorsScope()
{
#ifdef DEBUG
	return {LOGGER_COLOR_ERROR, LOGGER_FORMAT_ERROR_STARTWITH};
#else
	return {LOGGER_COLOR_ERROR};
#endif
}
