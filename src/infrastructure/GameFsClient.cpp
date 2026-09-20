#include "infrastructure/GameFsClient.hpp"

#include <algorithm>
#include <iterator>
#include <vector>
namespace fmm::infrastructure {

auto GameFsClient::applyPlan(const core::GameFsPlan& plan) const -> bool {
  if (handle_ == nullptr) {
    return false;
  }

  std::vector<const char*> file_pointers;
  file_pointers.reserve(plan.deployed_files.size());
  std::ranges::transform(plan.deployed_files, std::back_inserter(file_pointers),
                         [](const std::string& file) -> const char* { return file.c_str(); });

  FmmStringArray array{};
  array.data = file_pointers.empty() ? nullptr : file_pointers.data();
  array.length = file_pointers.size();

  FmmGameFsPlan c_plan{};
  c_plan.profile_id = plan.profile_id.c_str();
  c_plan.target_game_directory = plan.target_game_directory.c_str();
  c_plan.deployed_files = array;

  return fmm_gamefs_apply_plan(handle_, &c_plan) == 0;
}

} // namespace fmm::infrastructure
