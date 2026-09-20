#pragma once

extern "C" {
struct FmmContext;
FmmContext* fmm_context_create();
void fmm_context_destroy(FmmContext* ctx);
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

  GameFsClient& operator=(GameFsClient&& other) noexcept {
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
  GameFsClient& operator=(const GameFsClient&) = delete;

private:
  FmmContext* handle_{};
};

} // namespace fmm::infrastructure
