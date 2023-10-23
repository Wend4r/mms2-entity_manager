#include "logger.h"

#include "entity_manager.h"

#include <stdarg.h>
#include <tier0/logging.h>

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

constexpr auto DBG_Msg = Msg;
constexpr auto DBG_Warning = Warning;
constexpr auto DBG_Error = Error;
constexpr void (*DBG_DevMsg)(int, const tchar *, ...) = DevMsg;
constexpr void (*DBG_DevWarning)(int, const tchar *, ...) = DevWarning;

void EntityManager::Logger::Message(const char *pszContent)
{
	DBG_Msg(FORMAT_ENTITY_MANAGER_MESSAGE, g_pEntityManager->GetLogTag(), pszContent);
}

void EntityManager::Logger::MessageFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Message((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::Warning(const char *pszContent)
{
	DBG_Warning(FORMAT_ENTITY_MANAGER_WARNING, g_pEntityManager->GetLogTag(), pszContent);
}

void EntityManager::Logger::WarningFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Warning((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::Error(const char *pszContent)
{
	DBG_Error(FORMAT_ENTITY_MANAGER_ERROR, g_pEntityManager->GetLogTag(), pszContent);
}

void EntityManager::Logger::ErrorFormat(const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->Error((const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::DevMessage(int iLevel, const char *pszContent)
{
	DBG_DevMsg(iLevel, FORMAT_ENTITY_MANAGER_DEV_MESSAGE, g_pEntityManager->GetLogTag(), iLevel, pszContent);
}

void EntityManager::Logger::DevMessageFormat(int iLevel, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->DevMessage(iLevel, (const char *)this->m_sFormatBuffer);
}

void EntityManager::Logger::DevWarning(int iLevel, const char *pszContent)
{
	DBG_DevWarning(iLevel, FORMAT_ENTITY_MANAGER_DEV_WARNING, g_pEntityManager->GetLogTag(), iLevel, pszContent);
}

void EntityManager::Logger::DevWarningFormat(int iLevel, const char *pszFormat, ...)
{
	va_list aParams;

	va_start(aParams, pszFormat);
	V_vsnprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), pszFormat, aParams);
	va_end(aParams);

	this->DevWarning(iLevel, (const char *)this->m_sFormatBuffer);
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

EntityManager::Logger::Scope EntityManager::Logger::CreateMessagesScope()
{
	snprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), FORMAT_ENTITY_MANAGER_MESSAGE_STARTWITH, g_pEntityManager->GetLogTag());

	return {(const char *)this->m_sFormatBuffer};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateWarningsScope()
{
	snprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), FORMAT_ENTITY_MANAGER_WARNING_STARTWITH, g_pEntityManager->GetLogTag());

	return {(const char *)this->m_sFormatBuffer};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateErrorsScope()
{
	snprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), FORMAT_ENTITY_MANAGER_ERROR_STARTWITH, g_pEntityManager->GetLogTag());

	return {(const char *)this->m_sFormatBuffer};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateDevMessagesScope(int iLevel)
{
	snprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), FORMAT_ENTITY_MANAGER_DEV_MESSAGE_STARTWITH, g_pEntityManager->GetLogTag(), iLevel);

	return {(const char *)this->m_sFormatBuffer};
}

EntityManager::Logger::Scope EntityManager::Logger::CreateDevWarningsScope(int iLevel)
{
	snprintf((char *)this->m_sFormatBuffer, sizeof(this->m_sFormatBuffer), FORMAT_ENTITY_MANAGER_DEV_WARNING_STARTWITH, g_pEntityManager->GetLogTag(), iLevel);

	return {(const char *)this->m_sFormatBuffer};
}
