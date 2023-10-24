#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_LOGGER_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_LOGGER_

#include <stddef.h>
#include <functional>
#include <string>

#include <tier0/dbg.h>
#include <tier0/logging.h>

namespace EntityManager
{
	class Logger
	{
	public:
		Logger();

		void Message(const char *pszContent);
		void MessageFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);

		void Warning(const char *pszContent);
		void WarningFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);

		void ThrowAssert(const char *pszFilename, int iLine, const char *pszContent);
		void ThrowAssertFormat(const char *pszFilename, int iLine, const char *pszFormat, ...) FMTFUNCTION(4, 5);

		void Error(const char *pszContent);
		void ErrorFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);

	public:
		class Scope
		{
		public:
			typedef std::function<void (const char *)> SendFunc;

			Scope(const char *pszStartWith = "");
			Scope(const char *pszStartWith, const char *pszEnd);

			size_t Count();

			size_t Push(const char *pszContent);
			size_t PushFormat(const char *pszFormat, ...);
			size_t Send(SendFunc funcOn);

			class Message
			{
			public:
				const std::string &Get() const;
				size_t SetWithCopy(const char *pszContent);

			private:
				std::string m_sContent;
			};

		private:
			char m_sFormatBuffer[MAX_LOGGING_MESSAGE_LENGTH];

			std::string m_aStartWith;
			std::vector<Message> m_vec;
			std::string m_aEnd;
		};

		Scope CreateMessagesScope();
		Scope CreateWarningsScope();
		Scope CreateAssertScope();
		Scope CreateErrorsScope();

	private:
		char m_sFormatBuffer[MAX_LOGGING_MESSAGE_LENGTH];

		LoggingChannelID_t m_nChannelID;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_LOGGER_
