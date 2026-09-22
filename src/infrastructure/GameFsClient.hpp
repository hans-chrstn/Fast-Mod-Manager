#pragma once

#include "core/LegacyGameFsAbiPlan.hpp"

extern "C" {
struct FmmContext;

struct FmmStringArray {
  const char* const* data;
  size_t length;
};

struct FmmGameFsPlan {
  const char* profile_id;
  const char* target_game_directory;
  FmmStringArray deployed_files;
};

auto fmm_context_create() -> FmmContext*;
void fmm_context_destroy(FmmContext* ctx);
auto fmm_gamefs_apply_plan(FmmContext* ctx, const FmmGameFsPlan* plan) -> int;
}

namespace fmm::infrastructure {

class GameFsClient {
public:
  GameFsClient() : handle_(fmm_context_create()) {}
  ~GameFsClient() {
    if (handle_ != nullptr) {
      fmm_context_destroy(handle_);
      handle_ = nullptr;
    }
  }

  GameFsClient(GameFsClient&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }

  auto operator=(GameFsClient&& other) noexcept -> GameFsClient& {
    if (this != &other) {
      if (handle_ != nullptr) {
        fmm_context_destroy(handle_);
      }
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  GameFsClient(const GameFsClient&) = delete;
  auto operator=(const GameFsClient&) -> GameFsClient& = delete;

  [[nodiscard]] auto applyPlan(const core::LegacyGameFsAbiPlan& plan) const -> bool;

private:
  FmmContext* handle_{};
};

} // namespace fmm::infrastructure
