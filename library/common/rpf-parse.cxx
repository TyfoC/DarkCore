#include "rpf-parser.hxx"

//	Possible templates:
//	KEY = VALUE
//	KEY VALUE
List<RPFParser::Element> RPFParser::Parse(const char* data, size_t bytesCount) {
	List<Element> elements = List<Element>();
	Element tmp = {};

	size_t prevOffset = 0, nextOffset, keyLength, valueLength;
	while (prevOffset < bytesCount) {
		while ((data[prevOffset] == '\r' || data[prevOffset] == '\n' ||
			data[prevOffset] == ' ' || data[prevOffset] == '\t') && prevOffset < bytesCount) ++prevOffset;
		if (prevOffset >= bytesCount) break;
		nextOffset = prevOffset;
		while (data[nextOffset] != ' ' && data[nextOffset] != '\t' &&
			data[nextOffset] != '\r' && data[nextOffset] != '\n' && nextOffset < bytesCount) ++nextOffset;

		keyLength = nextOffset - prevOffset;
		tmp.Key = new char[keyLength + 1];
		MemoryUtils::Copy(tmp.Key, &data[prevOffset], keyLength);
		tmp.Key[keyLength] = 0;

		prevOffset = nextOffset + 1;
		if (prevOffset >= bytesCount) break;

		while ((data[prevOffset] == '\r' || data[prevOffset] == '\n' ||
			data[prevOffset] == ' ' || data[prevOffset] == '\t') && prevOffset < bytesCount) ++prevOffset;
		if (prevOffset >= bytesCount) break;
		else if (data[prevOffset] == '=') ++prevOffset;

		while ((data[prevOffset] == '\r' || data[prevOffset] == '\n' ||
			data[prevOffset] == ' ' || data[prevOffset] == '\t') && prevOffset < bytesCount) ++prevOffset;
		if (prevOffset > bytesCount) break;
		nextOffset = prevOffset;
		while (data[nextOffset] != '\r' && data[nextOffset] != '\n' && nextOffset < bytesCount) ++nextOffset;
		
		valueLength = nextOffset - prevOffset;
		tmp.Value = new char[valueLength];
		MemoryUtils::Copy(tmp.Value, &data[prevOffset], valueLength);
		tmp.Value[valueLength] = 0;

		elements.Append(tmp);

		prevOffset = nextOffset + 1;
		tmp.Key = tmp.Value = 0;
	}

	return elements;
}

void RPFParser::FreeElements(List<Element>& elements) {
	const size_t count = elements.GetCount();
	for (size_t i = 0; i < count; i++) {
		if (elements[i].Key) delete[] elements[i].Key;
		if (elements[i].Value) delete[] elements[i].Value;
	}
}