#include <fstream>
#include <iostream>
#include <optional>

#pragma pack(push, 1)
struct ddfs_bs
{
  uint16_t sector_size;
  uint8_t sectors_per_cluster;
  uint32_t number_of_clusters;
};
#pragma pack(pop)

std::ostream& operator<<(std::ostream& out, const ddfs_bs& bs)
{
  out << "sector_size: " << bs.sector_size
      << ", sectors_per_cluster: " << (unsigned)bs.sectors_per_cluster
      << ", number_of_clusters: " << bs.number_of_clusters;
  return out;
}

class ddfs
{
public:
  explicit ddfs(const char* img_path)
    : m_img{ img_path,
             std::ios_base::binary | std::fstream::in | std::fstream::out }
  {
  }

  bool is_open() const { return m_img.is_open(); }

  std::optional<ddfs_bs> read_bs()
  {
    if (!is_open()) {
      return std::nullopt;
    }

    ddfs_bs bs;
    read(0, sizeof(bs), &bs);
    return bs;
  }

  void write_bs(const ddfs_bs& bs) { write(0, sizeof(bs), &bs); }

private:
  void set_read_offset(std::fstream::pos_type offset)
  {
    m_img.seekg(offset, std::ios_base::beg);
  }

  void set_write_offset(std::fstream::pos_type offset)
  {
    m_img.seekp(offset, std::ios_base::beg);
  }

  void read(std::fstream::pos_type offset, unsigned size, void* dest)
  {
    set_read_offset(offset);
    m_img.read(reinterpret_cast<char*>(dest), size);
  }

  void write(std::fstream::pos_type offset, unsigned size, const void* data)
  {
    set_write_offset(offset);
    m_img.write(reinterpret_cast<const char*>(data), size);
  }

private:
  std::fstream m_img;
};

int main(int argc, const char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: mkddfs <path to img>\n";
    return 1;
  }

  std::cout << "Image path: " << argv[1] << "\n";

  ddfs fs{ argv[1] };

  if (!fs.is_open()) {
    std::cout << "Could not open fs\n";
    return 1;
  }

  const auto boot_sector = fs.read_bs();
  if (!boot_sector.has_value()) {
    std::cout << "Could not read bs\n";
    return 1;
  }

  std::cout << "current boot_sector: " << *boot_sector << "\n";

  std::optional<ddfs_bs> bs = ddfs_bs{ .sector_size = 512,
                                       .sectors_per_cluster = 1,
                                       .number_of_clusters = 4 };

  std::cout << "writing boot_sector: " << *bs << "\n";
  fs.write_bs(*bs);
}
