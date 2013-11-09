#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fs_attr.h>
#include <File.h>

// fs_remove_attr(int fd, const char *attribute)

// fs_write_attr(int fd, const char* attribute, uint32 type, off_t pos, 
// const void *buffer, size_t count)

int main(int argc, char *args[]) {
	if (argc != 5) {
		fprintf(stderr, "%s source_file type attribute target_file\n", args[0]);
		return 1;
	}

	const char* source = args[1];
	const char* type = args[2];
	const char* attribute = args[3];
	const char* target = args[4];
	if (strlen(type) != 4) {
		fprintf(stderr, "type must contain four characters\n", type);
		return 1;
	}
	
	type_code typeCode = 0;
	for (int i = 0; i < 4; i ++) {
		typeCode <<= 8;
		typeCode |= (unsigned char)type[i];			
	}
	
	BFile sourceFile(source, B_READ_ONLY);
	if (sourceFile.InitCheck() != B_OK) {
		fprintf(stderr, "Could not open file %s\n", source);
		return 1;
	}
	
	BFile targetFile(target, B_READ_ONLY);
	if (targetFile.InitCheck() != B_OK) {
		fprintf(stderr, "Could not open file %s\n", target);
		return 1;
	}
	
	off_t size;
	sourceFile.GetSize(&size);
	uint8 buffer[size];
	if (size != sourceFile.Read(buffer, size)) {
		fprintf(stderr, "Error reading file %s\n", source);
		return 1;
	}
	
	if (size != targetFile.WriteAttr(attribute, typeCode, 0, buffer, size)) {
		fprintf(stderr, "Error writing attribute %s to %s\n", attribute, target);
		return 1;
	}
	
}

