#ifndef CATTA_UTIL_FILEUTIL_H
#define CATTA_UTIL_FILEUTIL_H

#include <string>

namespace catta {

class FileReader {
public:
	static int readFile(const char* filename,
			std::string& content,
			size_t size) {
		FileReader file(filename);
		return file.readToString(content, size);
	}

public:
	// noncopyable
	FileReader(const FileReader&) = delete;
	FileReader& operator=(const FileReader&) = delete;

public:
	FileReader(const char* filename);
	~FileReader();

	int readToString(std::string& content, size_t size);

private:
	int fd_;
	int err_;
}; // end class FileReader

} // end namespace catta

#endif // CATTA_UTIL_FILEUTIL_H
