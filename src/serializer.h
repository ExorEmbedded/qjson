#pragma once

#include "qjson-backport_global.h"
#include "qjsondocument.h"

namespace QJson
{

	class Q_JSONRPC_EXPORT Serializer
	{
	public:
		Serializer() {}
		~Serializer() {}

		QByteArray serialize(const QVariant& v, bool* ok = NULL)
		{
			if (ok) *ok = true; // reset to false on error
			const QJsonDocument& doc = QJsonDocument::fromVariant(v);
			if (!doc.isNull()) {
				return doc.toJson();
			}
			return valueToJson(v, ok);
		}
	private:
		QByteArray valueToJson(const QVariant &v, bool* ok);

	};

}
