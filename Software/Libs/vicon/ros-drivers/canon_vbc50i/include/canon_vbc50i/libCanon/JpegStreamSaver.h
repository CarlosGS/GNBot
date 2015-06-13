#ifndef JPEG_STREAM_SAVER_H
#define JPEG_STREAM_SAVER_H

#include <string>

class JpegStreamSaver
{
	protected:
		std::string directory;
		std::string base;
		unsigned int counter;

	public:
		JpegStreamSaver() : directory("/tmp"), base("stream") {counter = 0;}
		JpegStreamSaver(const std::string & dir,
				const std::string & b) :
			directory(dir), base(b) { counter = 0;}
		~JpegStreamSaver() {}

		void setDirectory(const std::string & dir) {
			directory = dir;
		}

		void setBase(const std::string & b) {
			base = b;
		}

		bool record(const char * buffer, unsigned int size);

};

#endif // JPEG_STREAM_SAVER_H
