/**
 *  Copyright 2014-2015 MongoDB, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

#include "functions.h"

#include "../../../bson.h"

extern "C" {
#include "../../../libbson/src/bson/bson.h"
}

namespace HPHP {

String HHVM_FUNCTION(MongoDBBsonFromPHP, const Variant &data)
{
	bson_t *bson;
	String s;
	unsigned char *data_s;

	VariantToBsonConverter converter(data, HIPPO_BSON_NO_FLAGS);
	bson = bson_new();
	converter.convert(bson);

	s = String(bson->len, ReserveString);
	data_s = (unsigned char*) s.bufferSlice().ptr;
	memcpy(data_s, bson_get_data(bson), bson->len);
	s.setSize(bson->len);

	return s;
}

Variant HHVM_FUNCTION(MongoDBBsonFromJson, const String &data)
{
	bson_t bson = BSON_INITIALIZER;
	bson_error_t error;

	if (bson_init_from_json(&bson, (const char *)data.c_str(), data.length(), &error)) {
		String s;
		unsigned char *data_s;

		s = String(bson.len, ReserveString);
		data_s = (unsigned char*) s.bufferSlice().ptr;
		memcpy(data_s, bson_get_data(&bson), bson.len);
		s.setSize(bson.len);

		return s;
	} else {
		return Variant();
	}
}

Variant HHVM_FUNCTION(MongoDBBsonToPHP, const String &data, const Variant &typemap)
{
	Variant v;
	hippo_bson_conversion_options_t options = { HIPPO_TYPEMAP_ARRAY, HIPPO_TYPEMAP_STDCLASS };

	BsonToVariantConverter convertor((const unsigned char*) data.c_str(), data.length(), options);
	convertor.convert(&v);

	return Variant(v.toArray());
}

Variant HHVM_FUNCTION(MongoDBBsonToJson, const String &data)
{
	const bson_t  *b;
	bson_reader_t *reader;
	String         s;

	reader = bson_reader_new_from_data((const unsigned char *)data.c_str(), data.length());
	b = bson_reader_read(reader, NULL);

	if (b) {
		char   *str;
		size_t  str_len;
		unsigned char *data_s;

		str = bson_as_json(b, &str_len);

		s = String(str_len, ReserveString);
		data_s = (unsigned char*) s.bufferSlice().ptr;
		memcpy(data_s, str, str_len);
		s.setSize(str_len);

		bson_free(str);
	} else {
		return Variant();
	}
	bson_reader_destroy(reader);

	return Variant(s);
}


}
