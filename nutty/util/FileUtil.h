#ifndef NUTTY_UTIL_FILEUTIL_H
#define NUTTY_UTIL_FILEUTIL_H

#include <string>

namespace nutty {

class FileReader {
public:
	static int readFile(const char* filename,
			std::string& content,
			size_t size) {
		FileReader file(filename);
		return file.readToString(content, size);
	}

public:
	FileReader(const char* filename);
	~FileReader();

	FileReader(const FileReader&) = delete;
	FileReader& operator=(const FileReader&) = delete;

	int readToString(std::string& content, size_t size);

private:
	int fd_;
	int err_;
}; // end class FileReader

} // end namespace nutty

#endif // NUTTY_UTIL_FILEUTIL_H
