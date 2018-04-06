#pragma once

#include "qjson-backport_global.h"
#include "qjsondocument.h"

namespace QJson
{

	class Q_JSONRPC_EXPORT Parser
	{
	public:
		Parser() {}
		~Parser() {}
		QVariant parse(const QByteArray& data, bool* ok = NULL);

		QVariant parse(QIODevice* dev, bool* ok = NULL)
		{
			dev->open(QIODevice::ReadOnly);
			QVariant v = parse(dev->readAll(), ok);
			dev->close();
			return v;
		}

	private:
		QVariant parseValue(const char* start, const char* end, bool* ok);
		QVariant parseString(const char* start, const char* end, bool* ok);
		QVariant parseNumber(const char* start, const char* end, bool* ok);
		QJsonDocument doc;
	};

}
