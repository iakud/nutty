#include <nutty/util/FileUtil.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace nutty;

FileReader::FileReader(const char* filename)
	: fd_(::open(filename, O_RDONLY | O_CLOEXEC))
	, err_(0) {
	if (fd_ < 0) {
		err_ = errno;
	}
}

FileReader::~FileReader() {
	if (fd_ >= 0) {
		::close(fd_);
	}
}

int FileReader::readToString(std::string& content, size_t size) {
	if (fd_ < 0) {
		return err_;
	}

	char buf[65536];
	while (content.size() < size) {
		size_t toRead = std::min(size - content.size(), sizeof buf);
		ssize_t nRead = ::read(fd_, buf, toRead);
		if (nRead > 0) {
			content.append(buf, nRead);
		} else {
			if (nRead < 0) {
				return errno;	
			}
			break;
		}
	}
	return err_;
}
