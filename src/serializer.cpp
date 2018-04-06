#pragma once

#include "serializer.h"

namespace QJson
{
	static inline bool isUnicodeNonCharacter(uint ucs4)
	{
		return (ucs4 & 0xfffe) == 0xfffe
			|| (ucs4 - 0xfdd0U) < 16;
	}

	static inline uchar hexdig(uint u)
	{
		return (u < 0xa ? '0' + u : 'a' + u - 0xa);
	}

	static QByteArray escapedString(const QString &s)
	{
		const uchar replacement = '?';
		QByteArray ba(s.length(), Qt::Uninitialized);

		uchar *cursor = (uchar *)ba.data();
		const uchar *ba_end = cursor + ba.length();

		const QChar *ch = (const QChar *)s.constData();
		const QChar *end = ch + s.length();

		int surrogate_high = -1;

		while (ch < end) {
			if (cursor >= ba_end - 6) {
				// ensure we have enough space
				int pos = cursor - (const uchar *)ba.constData();
				ba.resize(ba.size()*2);
				cursor = (uchar *)ba.data() + pos;
				ba_end = (const uchar *)ba.constData() + ba.length();
			}

			uint u = ch->unicode();
			if (surrogate_high >= 0) {
				if (ch->isLowSurrogate()) {
					u = QChar::surrogateToUcs4(surrogate_high, u);
					surrogate_high = -1;
				} else {
					// high surrogate without low
					*cursor = replacement;
					++ch;
					surrogate_high = -1;
					continue;
				}
			} else if (ch->isLowSurrogate()) {
				// low surrogate without high
				*cursor = replacement;
				++ch;
				continue;
			} else if (ch->isHighSurrogate()) {
				surrogate_high = u;
				++ch;
				continue;
			}

			if (u < 0x80) {
				if (u < 0x20 || u == 0x22 || u == 0x5c) {
					*cursor++ = '\\';
					switch (u) {
				case 0x22:
					*cursor++ = '"';
					break;
				case 0x5c:
					*cursor++ = '\\';
					break;
				case 0x8:
					*cursor++ = 'b';
					break;
				case 0xc:
					*cursor++ = 'f';
					break;
				case 0xa:
					*cursor++ = 'n';
					break;
				case 0xd:
					*cursor++ = 'r';
					break;
				case 0x9:
					*cursor++ = 't';
					break;
				default:
					*cursor++ = 'u';
					*cursor++ = '0';
					*cursor++ = '0';
					*cursor++ = hexdig(u>>4);
					*cursor++ = hexdig(u & 0xf);
					}
				} else {
					*cursor++ = (uchar)u;
				}
			} else {
				if (u < 0x0800) {
					*cursor++ = 0xc0 | ((uchar) (u >> 6));
				} else {
					// is it one of the Unicode non-characters?
					if (isUnicodeNonCharacter(u)) {
						*cursor++ = replacement;
						++ch;
						continue;
					}

					if (u > 0xffff) {
						*cursor++ = 0xf0 | ((uchar) (u >> 18));
						*cursor++ = 0x80 | (((uchar) (u >> 12)) & 0x3f);
					} else {
						*cursor++ = 0xe0 | (((uchar) (u >> 12)) & 0x3f);
					}
					*cursor++ = 0x80 | (((uchar) (u >> 6)) & 0x3f);
				}
				*cursor++ = 0x80 | ((uchar) (u&0x3f));
			}
			++ch;
		}

		ba.resize(cursor - (const uchar *)ba.constData());
		return ba;
	}

	QByteArray Serializer::valueToJson(const QVariant &v, bool* ok)
	{
		if (ok) *ok = true;
		switch (v.type()) {
		case QVariant::Int:
			return QByteArray::number(v.toInt());
			break;
		case QVariant::UInt:
			return QByteArray::number(v.toUInt());
			break;
		case QVariant::LongLong:
			return QByteArray::number(v.toLongLong());
			break;
		case QVariant::ULongLong:
			return QByteArray::number(v.toULongLong());
			break;
		case QVariant::Double:
			return QByteArray::number(v.toDouble());
			break;
		case QVariant::Char:
			return QByteArray::number((unsigned short)v.toChar().toLatin1());
			break;
		case QVariant::Bool:
			return QByteArray::number(v.toBool());
			break;
		case QMetaType::ULong:
			return QByteArray::number((unsigned int)v.value<unsigned long>());
			break;
		case QMetaType::Long:
			return QByteArray::number((int)v.value<long>());
			break;
		case QMetaType::UShort:
			return QByteArray::number(v.value<unsigned short>());
			break;
		case QMetaType::UChar:
			return QByteArray::number(v.value<unsigned char>());
			break;
		case QMetaType::Char:
			return QByteArray::number(v.value<char>());
			break;
		case QMetaType::Short:
			return QByteArray::number(v.value<short>());
			break;
		case QMetaType::Float:
			return QByteArray::number(v.value<float>());
			break;
		case QVariant::String:
			return "\"" +  escapedString(v.toString()) + "\"";
		default:
			if (ok) *ok = false;
			return "null";
		}
	}

}
