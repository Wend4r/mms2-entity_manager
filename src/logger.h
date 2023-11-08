#ifndef _INCLUDE_LOGGER_H_
#define _INCLUDE_LOGGER_H_

#include <stddef.h>
#include <functional>
#include <string>

#include <tier0/dbg.h>
#include <tier0/logging.h>

#define LOGGER_FORMAT_DETAILED_STARTWITH "DEV: "
#define LOGGER_FORMAT_DETAILED_STARTWITH_PREFIX "[%s] " LOGGER_FORMAT_DETAILED_STARTWITH_ADDITIONAL
#define LOGGER_FORMAT_DETAILED LOGGER_FORMAT_DETAILED_STARTWITH_PREFIX "%s\n"
#define LOGGER_COLOR_DETAILED {255, 255, 255, 191}

#define LOGGER_FORMAT_MESSAGE_STARTWITH ""
#define LOGGER_FORMAT_MESSAGE_STARTWITH_PREFIX "[%s] " LOGGER_FORMAT_MESSAGE_STARTWITH
#define LOGGER_FORMAT_MESSAGE LOGGER_FORMAT_MESSAGE_STARTWITH_PREFIX "%s\n"
#define LOGGER_COLOR_MESSAGE {255, 255, 255, 255}

#define LOGGER_FORMAT_WARNING_STARTWITH "WARNING: "
#define LOGGER_FORMAT_WARNING_STARTWITH_PREIFIX "[%s] " LOGGER_FORMAT_WARNING_STARTWITH
#define LOGGER_FORMAT_WARNING LOGGER_FORMAT_WARNING_STARTWITH_PREIFIX "%s\n"
#define LOGGER_COLOR_WARNING {255, 255, 255, 255}

#define LOGGER_FORMAT_ASSERT_STARTWITH "ASSERT: "
#define LOGGER_FORMAT_ASSERT_STARTWITH_PREIFIX "[%s] " LOGGER_FORMAT_ASSERT_STARTWITH
#define LOGGER_FORMAT_ASSERT LOGGER_FORMAT_ASSERT_STARTWITH_PREIFIX "%s\n"
#define LOGGER_COLOR_ASSERT {255, 127, 0, 255}

#define LOGGER_FORMAT_ERROR_STARTWITH "ERROR: "
#define LOGGER_FORMAT_ERROR_STARTWITH_PREIFIX "[%s] " LOGGER_FORMAT_ERROR_STARTWITH
#define LOGGER_FORMAT_ERROR LOGGER_FORMAT_ERROR_STARTWITH_PREIFIX "%s\n"
#define LOGGER_COLOR_ERROR {255, 0, 0, 255}

class Logger
{
public:
	Logger(const char *pszName, RegisterTagsFunc pfnRegisterTagsFunc, int iFlags = 0, LoggingVerbosity_t eVerbosity = LV_DEFAULT, const Color &aDefault = UNSPECIFIED_LOGGING_COLOR);

	void Detailed(const char *pszContent);
	void Detailed(Color aColor, const char *pszContent);
	void DetailedFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);
	void DetailedFormat(Color aColor, const char *pszFormat, ...) FMTFUNCTION(3, 4);

	void Message(const char *pszContent);
	void Message(Color aColor, const char *pszContent);
	void MessageFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);
	void MessageFormat(Color aColor, const char *pszFormat, ...) FMTFUNCTION(3, 4);

	void Warning(const char *pszContent);
	void Warning(Color aColor, const char *pszContent);
	void WarningFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);
	void WarningFormat(Color aColor, const char *pszFormat, ...) FMTFUNCTION(3, 4);

	void ThrowAssert(const char *pszFilename, int iLine, const char *pszContent);
	void ThrowAssert(const char *pszFilename, int iLine, Color aColor, const char *pszContent);
	void ThrowAssertFormat(const char *pszFilename, int iLine, const char *pszFormat, ...) FMTFUNCTION(4, 5);
	void ThrowAssertFormat(const char *pszFilename, int iLine, Color aColor, const char *pszFormat, ...) FMTFUNCTION(5, 6);

	void Error(const char *pszContent);
	void Error(Color aColor, const char *pszContent);
	void ErrorFormat(const char *pszFormat, ...) FMTFUNCTION(2, 3);
	void ErrorFormat(Color aColor, const char *pszFormat, ...) FMTFUNCTION(3, 4);

	void DoTests();

public:
	class Scope
	{
	public:
		typedef std::function<void (const char *)> SendFunc;
		typedef std::function<void (const Color &, const char *)> SendColorFunc;

		Scope(const Color &rgbaInit, const char *pszStartWith = "");
		Scope(const Color &rgbaInit, const char *pszStartWith, const char *pszEnd);

		const Color &GetColor() const;
		size_t Count();

		void SetColor(const Color &rgba);

		size_t Push(const char *pszContent);
		size_t PushFormat(const char *pszFormat, ...);

		size_t Send(SendFunc funcOn);
		size_t SendColor(SendColorFunc funcOn);

		class Message
		{
		public:
			Message(const Color &rgbaInit);

			const Color &GetColor() const;
			const std::string &Get() const;
			size_t SetWithCopy(const char *pszContent);

		private:
			Color m_aColor;
			std::string m_sContent;
		};

	private:
		char m_sFormatBuffer[MAX_LOGGING_MESSAGE_LENGTH];

		Color m_aColor;

		std::string m_aStartWith;
		std::vector<Message> m_vec;
		std::string m_aEnd;
	};

	Scope CreateDetailsScope();
	Scope CreateMessagesScope();
	Scope CreateWarningsScope();
	Scope CreateAssertScope();
	Scope CreateErrorsScope();

private:
	char m_sFormatBuffer[MAX_LOGGING_MESSAGE_LENGTH];

	LoggingChannelID_t m_nChannelID;
};

#endif // _INCLUDE_LOGGER_H_