#pragma once

#include <QMetaObject>
#include <QObject>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

namespace fmm::ui {

template <typename T> class AsyncTask {
public:
  AsyncTask(QObject* context, std::function<T(std::stop_token)> work,
            std::function<void(T)> on_success, std::function<void(std::string)> on_error)
      : m_thread([context, work = std::move(work), on_success = std::move(on_success),
                  on_error = std::move(on_error)](const std::stop_token& stoken) -> void {
          try {
            T result = work(stoken);
            if (!stoken.stop_requested()) {
              QMetaObject::invokeMethod(
                  context, [on_success, result]() -> void { on_success(result); },
                  Qt::QueuedConnection);
            }
          } catch (const std::exception& e) {
            if (!stoken.stop_requested()) {
              std::string msg = e.what();
              QMetaObject::invokeMethod(
                  context, [on_error, msg]() -> void { on_error(msg); }, Qt::QueuedConnection);
            }
          } catch (...) {
            if (!stoken.stop_requested()) {
              QMetaObject::invokeMethod(
                  context, [on_error]() -> void { on_error("Unknown error occurred"); },
                  Qt::QueuedConnection);
            }
          }
        }) {}

  ~AsyncTask() { cancel(); }

  void cancel() { m_thread.request_stop(); }

private:
  std::jthread m_thread;
};

} // namespace fmm::ui
