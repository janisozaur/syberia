#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

struct entry {
	std::string name;
	uint32_t offset;
	uint32_t size;
	entry(const std::string name, uint32_t offset, uint32_t size) : name(name), offset(offset), size(size) {}
};

int main(int argc, const char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		printf("arg %d = '%s'\n", i, argv[i]);
	}
	if (argc < 2)
	{
		printf("File is required\n");
	}
	std::string archive_name = argv[1];
	fs::path archive_path(archive_name);
	std::ifstream file(archive_name, std::ios::binary);
	if (!file.is_open())
	{
		printf("failed to open '%s'\n", archive_name.c_str());
		return -1;
	}

    file.seekg(0, std::ios::end);
	uint32_t archive_size = file.tellg();
    file.seekg(0, std::ios::beg);
	printf("size is %u\n", archive_size);
	uint32_t vxbg;
	file.read((char *)&vxbg, 4);
	printf("vxbg = 0x%x\n", vxbg); // 0x47425856
	uint32_t total = 8;
	std::vector<entry> entries;
	do {
		uint32_t offset;
		file.read((char *)&offset, 4);
		total += offset;
		std::string name;
		std::getline(file, name, '\0');
		entries.emplace_back(name, total, 0);
	} while (file.tellg() < entries[0].offset - 4);
	for (int i = 0; i < entries.size() - 1; i++)
	{
		entries[i].size = entries[i + 1].offset - entries[i].offset;
	}
	entries[entries.size() - 1].size = archive_size - entries.back().offset;
	std::string out_dir{archive_path.filename()};
	fs::create_directory(out_dir);
	for (const auto &entry : entries)
	{
		printf("file = '%s', offset = 0x%x, size = 0x%x\n", entry.name.c_str(), entry.offset, entry.size);
		std::ofstream out_file(out_dir + "/" + entry.name, std::ios::binary);
		file.seekg(entry.offset, std::ios::beg);
		uint32_t to_read = entry.size;
		char buf[8192];
		while (to_read > 0)
		{
			uint32_t chunk_size = std::min(to_read, (uint32_t)std::size(buf));
			file.read(buf, chunk_size);
			out_file.write(buf, chunk_size);
			to_read -= chunk_size;
		}
	}
	return 0;
}
