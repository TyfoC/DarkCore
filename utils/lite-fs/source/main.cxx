#include "../include/lite-fs.hxx"
#include <iostream>

void PrintHelp() {
	std::cout << 
		"Welcome to LiteFS utility." << std::endl <<
		"Usage:" << std::endl <<
		"\tlite-fs <action> <parameters> <output>" << std::endl <<
		"\tactions:" << std::endl <<
		"\t\t-h\t\tprint help" << std::endl <<
		"\t\t-gp [-a attributes] <partition name> <input directory> <output file path>\t\tgenerate LiteFS partition" << std::endl <<
		"\t\t-cpoc <partition path>\t\tcalculate objects count in LiteFS partition" << std::endl <<
		"\t\t-pp <partition path>\t\tprint partition" << std::endl <<
		"\t\t-gfsh <partition lba offset> <output file path>\t\tgenerate LiteFS header" << std::endl <<
		"List of attributes:" << std::endl <<
		"\t\ts - system" << std::endl <<
		"\t\tr - read only" << std::endl <<
		"\t\th - hidden" << std::endl <<
		"\t\tExample: lite-fs -genpart hs $STORAGE lite-fs-image.pbd" << std::endl <<
		"\t\t\tP.S.\tPBD - partition binary data" << std::endl <<
	std::endl;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		PrintHelp();
		return -1;
	}

	std::string action = std::string(argv[1]);
	if (action == "-h") PrintHelp();
	else if (action == "-gp") {
		uint64_t attributes = 0;
		std::string partName, inputPath, outputPath;
		if (!strcmp(argv[2], "-a")) {
			std::string userAttributes = std::string(argv[3]);

			for (size_t i = 0; userAttributes[i]; i++) {
				switch(userAttributes[i]) {
					case 's':
					case 'S':
						attributes |= LiteFS::OBJECT_ATTRIBUTE_SYSTEM;
						break;
					case 'r':
					case 'R':
						attributes |= LiteFS::OBJECT_ATTRIBUTE_READ_ONLY;
						break;
					case 'h':
					case 'H':
						attributes |= LiteFS::OBJECT_ATTRIBUTE_HIDDEN;
						break;
					default:
						std::cout << "Warning: attribute ignored: `" << userAttributes[i] << "`!" << std::endl;
						break;
				};
			}

			partName = std::string(argv[4]);
			inputPath = std::string(argv[5]);
			outputPath = std::string(argv[6]);
		}
		else {
			partName = std::string(argv[2]);
			inputPath = std::string(argv[3]);
			outputPath = std::string(argv[4]);
			attributes = LiteFS::OBJECT_ATTRIBUTE_SYSTEM;
		}

		std::vector<LiteFS::FileSystemObject> fsObjects = LiteFS::GetFileSystemObjects(inputPath);
		
		size_t outputSize = 0;
		uint8_t* data = LiteFS::GeneratePartition(partName, &outputSize, fsObjects, attributes);
		std::ofstream output(outputPath, std::ios::binary);
		output.write((char*)data, outputSize);
		output.close();
		LiteFS::FreeFileSystemObjects(fsObjects);
		delete[] data;
	}
	else if (action == "-cpoc") {
		std::ifstream input(argv[2], std::ios_base::binary);
		if (!input.is_open()) {
			std::cout << "Error: cannot open LiteFS partition file: `" << argv[2] << "`!" << std::endl;
			return -2;
		}

		std::stringstream ss;
		ss << input.rdbuf();
		std::string str = ss.str();

		const uint8_t* data = (uint8_t*)&str.c_str()[0];
		size_t objectsCount = LiteFS::GetPartitionObjectsCount(data);

		std::cout << "LiteFS partition contains " << objectsCount << " objects" << std::endl;
		input.close();
	}
	else if (action == "-pp") {
		std::ifstream input(argv[2], std::ios_base::binary);
		if (!input.is_open()) {
			std::cout << "Error: cannot open LiteFS partition file: `" << argv[2] << "`!" << std::endl;
			return -2;
		}

		std::stringstream ss;
		ss << input.rdbuf();
		std::string str = ss.str();

		const uint8_t* data = (uint8_t*)&str.c_str()[0];
		LiteFS::PrintPartition(data);

		input.close();
	}
	else if (action == "-gfsh") {
		const int64_t partLBA = std::stoull(argv[2]);
		const std::string outPath = std::string(argv[3]);

		LiteFS::FSHeader fsHeader = LiteFS::GenerateEmptyFSHeader();
		if (!LiteFS::AddFSHeaderPartition(fsHeader, partLBA)) {
			std::cout << "Error: max partitions count per LiteFS header: " << LiteFS::MaxPartitionsCount << std::endl;
			return -3;
		}

		std::ofstream output(outPath, std::ios::binary);
		if (!output.is_open()) {
			std::cout << "Error: cannot create/write LiteFS partition file: `" << outPath << "`!" << std::endl;
			return -4;
		}

		output.write((char*)&fsHeader, sizeof(LiteFS::FSHeader));
		output.close();
	}
}