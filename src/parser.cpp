#pragma once

#include "parser.h"

namespace QJson
{

	QVariant Parser::parse(const QByteArray& data, bool* ok)
	{
		if (ok) *ok = true; // reset to false on error

		// look for first character to check if if should be parsed 
		// as a json document or a basic type
		// skip spaces
		char c = ' ';
		int i=0;
		for(i=0; i<data.size(); i++) {
			c = data[i];
			if (c == ' ' || c == '\r' || c == '\t' || c == '\n')
				continue;
			break;
		}
		if (c == '[' || c == '{') {
			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(data, &error);
			if (ok) {
				*ok = error.error == QJsonParseError::NoError;
			}
			return doc.toVariant();
		} else {
			return parseValue(&data.constData()[i], &data.constData()[data.size()-i], ok);
		}
	}



	QVariant Parser::parseValue(const char* json, const char* end, bool* ok)
	{
		switch (*json++) {
	case 'n':
		break;  // either null or error, don't check in any case is QVariant()
	case 't':
		if (end - json >= 4) {
			if (*json++ == 'r' &&
				*json++ == 'u' &&
				*json++ == 'e') {
					return QVariant(true);
			}
		}
		break;
	case 'f':
		if (end - json >= 5) {
			if (*json++ == 'a' &&
				*json++ == 'l' &&
				*json++ == 's' &&
				*json++ == 'e') {
					return QVariant(false);
			}
		}
		break;
	case '\'':
	case '\"':
		return parseString(json, end, ok);
	default:
		--json;
		return parseNumber(json, end, ok);
		}

		if (ok) *ok = false;
		return QVariant();
	}

	QVariant Parser::parseNumber(const char* json, const char* end, bool* ok)
	{
		const char* start = json;
		bool isInt = true;

		// minus
		if (json < end && *json == '-')
			++json;

		// int = zero / ( digit1-9 *DIGIT )
		if (json < end && *json == '0') {
			++json;
		} else {
			while (json < end && *json >= '0' && *json <= '9')
				++json;
		}

		// frac = decimal-point 1*DIGIT
		if (json < end && *json == '.') {
			isInt = false;
			++json;
			while (json < end && *json >= '0' && *json <= '9')
				++json;
		}

		// exp = e [ minus / plus ] 1*DIGIT
		if (json < end && (*json == 'e' || *json == 'E')) {
			isInt = false;
			++json;
			if (json < end && (*json == '-' || *json == '+'))
				++json;
			while (json < end && *json >= '0' && *json <= '9')
				++json;
		}

		QByteArray number(start, json - start);

		if (isInt) {
			bool _ok;
			if (number.size() > 8) {
				// actually json does not support 64bit integers (only double)!!
				long long n = number.toLongLong(&_ok);
				if (_ok)
					return QVariant(n);
			} else {
				int n = number.toInt(&_ok);
				if (_ok)
					return QVariant(n);
			}
		}

		bool _ok;
		double d = number.toDouble(&_ok);
		if (_ok) {
			return QVariant(d);
		}

		if (ok) *ok = false;
		return QVariant();
	}

	static inline bool addHexDigit(char digit, uint *result)
	{
		*result <<= 4;
		if (digit >= '0' && digit <= '9')
			*result |= (digit - '0');
		else if (digit >= 'a' && digit <= 'f')
			*result |= (digit - 'a') + 10;
		else if (digit >= 'A' && digit <= 'F')
			*result |= (digit - 'A') + 10;
		else
			return false;
		return true;
	}

	static inline bool scanEscapeSequence(const char *&json, const char *end, uint *ch)
	{
		++json;
		if (json >= end)
			return false;

		uint escaped = *json++;
		switch (escaped) {
	case '"':
		*ch = '"'; break;
	case '\\':
		*ch = '\\'; break;
	case '/':
		*ch = '/'; break;
	case 'b':
		*ch = 0x8; break;
	case 'f':
		*ch = 0xc; break;
	case 'n':
		*ch = 0xa; break;
	case 'r':
		*ch = 0xd; break;
	case 't':
		*ch = 0x9; break;
	case 'u': {
		*ch = 0;
		if (json > end - 4)
			return false;
		for (int i = 0; i < 4; ++i) {
			if (!addHexDigit(*json, ch))
				return false;
			++json;
		}
		return true;
			  }
	default:
		*ch = escaped;
		return true;
		}
		return true;
	}

	static inline bool isUnicodeNonCharacter(uint ucs4)
	{
		return (ucs4 & 0xfffe) == 0xfffe
			|| (ucs4 - 0xfdd0U) < 16;
	}

	static inline bool scanUtf8Char(const char *&json, const char *end, uint *result)
	{
		int need;
		uint min_uc;
		uint uc;
		uchar ch = *json++;
		if (ch < 128) {
			*result = ch;
			return true;
		} else if ((ch & 0xe0) == 0xc0) {
			uc = ch & 0x1f;
			need = 1;
			min_uc = 0x80;
		} else if ((ch & 0xf0) == 0xe0) {
			uc = ch & 0x0f;
			need = 2;
			min_uc = 0x800;
		} else if ((ch&0xf8) == 0xf0) {
			uc = ch & 0x07;
			need = 3;
			min_uc = 0x10000;
		} else {
			return false;
		}

		if (json >= end - need)
			return false;

		for (int i = 0; i < need; ++i) {
			ch = *json++;
			if ((ch&0xc0) != 0x80)
				return false;
			uc = (uc << 6) | (ch & 0x3f);
		}

		if (isUnicodeNonCharacter(uc) || uc >= 0x110000 ||
			(uc < min_uc) || (uc >= 0xd800 && uc <= 0xdfff))
			return false;

		*result = uc;
		return true;
	}

	QVariant Parser::parseString(const char* json, const char* end, bool* ok)
	{
		QString out;
		while (json < end) {
			uint ch = 0;
			if (*json == '"')
				break;
			else if (*json == '\\') {
				if (!scanEscapeSequence(json, end, &ch)) {
					if (ok) *ok = false;
					return QVariant();
				}
			} else {
				if (!scanUtf8Char(json, end, &ch)) {
					if (ok) *ok = false;
					return QVariant();
				}
			}
			out.append(QChar(ch));
		}

		if (json >= end) {
			if (ok) *ok = false;
			return QVariant();
		}

		return QVariant(out);
	}

}
