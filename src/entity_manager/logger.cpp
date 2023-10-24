#include "logger.h"

#include "entity_manager.h"

#include <stdarg.h>

#define FORMAT_ENTITY_MANAGER_MESSAGE_STARTWITH "[%s]: "
#define FORMAT_ENTITY_MANAGER_MESSAGE FORMAT_ENTITY_MANAGER_MESSAGE_STARTWITH "%s\n"

#define FORMAT_ENTITY_MANAGER_WARNING_STARTWITH "[%s] WARNING: "
#define FORMAT_ENTITY_MANAGER_WARNING FORMAT_ENTITY_MANAGER_WARNING_STARTWITH "%s\n"

#define FORMAT_ENTITY_MANAGER_ERROR_STARTWITH "[%s] ERROR "
#define FORMAT_ENTITY_MANAGER_ERROR FORMAT_ENTITY_MANAGER_ERROR_STARTWITH "%s\n"

#define FORMAT_ENTITY_MANAGER_DEV_MESSAGE_STARTWITH "[%s] DEV L%d: "
#define FORMAT_ENTITY_MANAGER_DEV_MESSAGE FORMAT_ENTITY_MANAGER_DEV_MESSAGE_STARTWITH "%s\n"

#define FORMAT_ENTITY_MANAGER_DEV_WARNING_STARTWITH "[%s] DEV WARNING L%d: "
#define FORMAT_ENTITY_MANAGER_DEV_WARNING FORMAT_ENTITY_MANAGER_DEV_WARNING_STARTWITH "%s\n"

const Color ENTITY_MANAGER_LOGGINING_COLOR = {255, 125, 125, 255};

EntityManager::Logger::Logger()
{
	this->m_nChannelID = LoggingSystem_RegisterLoggingChannel(g_pEntityManager->GetName(), [](LoggingChannelID_t nTagChannelID) {
		LoggingSystem_AddTagToChannel(nTagChannelID, g_pEntityManager->GetLogTag());
	}, 0, LV_DEFAULT, ENTITY_MANAGER_LOGGINING_COLOR);
}

void EntityManager::Logger::Detailed(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_DETAILED, pszContent);
}

void EntityManager::Logger::Detailed(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_DETAILED, aColor, pszContent);
}

void EntityManager::Logger::DetailedFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Detailed((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::DetailedFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Detailed(aColor, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::Message(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_MESSAGE, pszContent);
}

void EntityManager::Logger::Message(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_MESSAGE, aColor, pszContent);
}

void EntityManager::Logger::MessageFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Message((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::MessageFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Message(aColor, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::Warning(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_WARNING, pszContent);
}

void EntityManager::Logger::Warning(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_WARNING, aColor, pszContent);
}

void EntityManager::Logger::WarningFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Warning((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::WarningFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Warning(aColor, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::ThrowAssert(const char *pszFilename, int iLine, const char *pszContent)
{
	LoggingSystem_Log(this->m_nChannelID, LS_ASSERT, "%s (%d) : %s", pszFilename, iLine, pszContent);
}

void EntityManager::Logger::ThrowAssert(const char *pszFilename, int iLine, Color aColor, const char *pszContent)
{
	LoggingSystem_Log(this->m_nChannelID, LS_ASSERT, aColor, "%s (%d) : %s", pszFilename, iLine, pszContent);
}

void EntityManager::Logger::ThrowAssertFormat(const char *pszFilename, int iLine, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->ThrowAssert(pszFilename, iLine, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::ThrowAssertFormat(const char *pszFilename, int iLine, Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->ThrowAssert(pszFilename, iLine, aColor, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::Error(const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_ERROR, pszContent);
}

void EntityManager::Logger::Error(Color aColor, const char *pszContent)
{
	LoggingSystem_LogDirect(this->m_nChannelID, LS_ERROR, aColor, pszContent);
}

void EntityManager::Logger::ErrorFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Error((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::ErrorFormat(Color aColor, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Error(aColor, (const char *)this->m_sFormatBuffer);
}

EntityManager::Logger::Scope::Scope(const char *pszStartWith)
{
	this->m_aStartWith = pszStartWith;
	this->m_aEnd = "\n";
}

EntityManager::Logger::Scope::Scope(const char *pszStartWith, const char *pszEnd)
{
	this->m_aStartWith = pszStartWith;
	this->m_aEnd = pszEnd;
}

size_t EntityManager::Logger::Scope::Count()
{
	return this->m_vec.size();
}

size_t EntityManager::Logger::Scope::Push(const char *pszContent)
{
	Message aMsg;

	size_t nStoredLength = aMsg.SetWithCopy(pszContent);

	this->m_vec.push_back(aMsg);

	return nStoredLength;
}

size_t EntityManager::Logger::Scope::PushFormat(const char *pszFormat, ...)
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

size_t EntityManager::Logger::Scope::Send(SendFunc funcOn)
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

const std::string &EntityManager::Logger::Scope::Message::Get() const
{
	return this->m_sContent;
}

size_t EntityManager::Logger::Scope::Message::SetWithCopy(const char *pszContent)
{
	this->m_sContent = pszContent;

	return this->m_sContent.size();
}

EntityManager::Logger::Scope EntityManager::Logger::CreateDetailsScope()
{
	return {};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateMessagesScope()
{
	return {};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateWarningsScope()
{
	return {};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateAssertScope()
{
	return {};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateErrorsScope()
{
	return {};
}
