#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace fmm::domain {

enum class PackageFormat : std::uint8_t { Unknown, Zip, SevenZip, Rar };

class PackageIdentity {
public:
  PackageIdentity(std::string name, PackageFormat format)
      : m_name(std::move(name)), m_format(format) {}

  [[nodiscard]] auto name() const -> std::string_view { return m_name; }
  [[nodiscard]] auto format() const -> PackageFormat { return m_format; }

private:
  std::string m_name;
  PackageFormat m_format;
};

} // namespace fmm::domain
