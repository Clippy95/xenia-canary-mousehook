/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/SR1.h"

#include "xenia/base/platform_win.h"
#include "xenia/cpu/processor.h"
#include "xenia/emulator.h"
#include "xenia/hid/hid_flags.h"
#include "xenia/hid/input_system.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xmodule.h"
#include "xenia/kernel/xthread.h"
#include "xenia/xbox.h"

using namespace xe::kernel;

DECLARE_double(sensitivity);
DECLARE_bool(invert_y);
DECLARE_bool(invert_x);
DECLARE_double(right_stick_hold_time_workaround);

const uint32_t kTitleIdSaintsRow1 = 0x545107D1;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  uint32_t x_address;
  std::string title_version;
  uint32_t y_address;
};

std::map<SaintsRow1Game::GameBuild, GameBuildAddrs> supported_builds{
    {SaintsRow1Game::GameBuild::Unknown, {NULL, "", NULL}},
    {SaintsRow1Game::GameBuild::SaintsRow1_TU1,
     {0x827f9af8, "1.0.1", 0x827F9B00}}};

SaintsRow1Game::~SaintsRow1Game() = default;

bool SaintsRow1Game::IsGameSupported() {
  if (kernel_state()->title_id() != kTitleIdSaintsRow1) {
    return false;
  }

  const std::string current_version =
      kernel_state()->emulator()->title_version();

  for (auto& build : supported_builds) {
    if (current_version == build.second.title_version) {
      game_build_ = build.first;
      return true;
    }
  }

  return false;
}

float SaintsRow1Game::DegreetoRadians(float degree) {
  return (float)(degree * (M_PI / 180));
}

float SaintsRow1Game::RadianstoDegree(float radians) {
  return (float)(radians * (180 / M_PI));
}

bool SaintsRow1Game::DoHooks(uint32_t user_index, RawInputState& input_state,
                            X_INPUT_STATE* out_state) {
  if (!IsGameSupported()) {
    return false;
  }

  if (supported_builds.count(game_build_) == 0) {
    return false;
  }

  auto now = std::chrono::steady_clock::now();
  auto elapsed_x = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_movement_time_x_)
                       .count();
  auto elapsed_y = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_movement_time_y_)
                       .count();

  // Declare static variables for last deltas
  static int last_x_delta = 0;
  static int last_y_delta = 0;

  const long long hold_time =
      static_cast<long long>(cvars::right_stick_hold_time_workaround);
  // Check for mouse movement and set thumbstick values
  if (input_state.mouse.x_delta != 0) {
    if (input_state.mouse.x_delta > 0) {
      out_state->gamepad.thumb_rx = SHRT_MAX;
    } else {
      out_state->gamepad.thumb_rx = SHRT_MIN;
    }
    last_movement_time_x_ = now;
    last_x_delta = input_state.mouse.x_delta;
  } else if (elapsed_x < hold_time) {  // hold time
    if (last_x_delta > 0) {
      out_state->gamepad.thumb_rx = SHRT_MAX;
    } else {
      out_state->gamepad.thumb_rx = SHRT_MIN;
    }
  }

  if (input_state.mouse.y_delta != 0) {
    if (input_state.mouse.y_delta > 0) {
      out_state->gamepad.thumb_ry = SHRT_MAX;
    } else {
      out_state->gamepad.thumb_ry = SHRT_MIN;
    }
    last_movement_time_y_ = now;
    last_y_delta = input_state.mouse.y_delta;
  } else if (elapsed_y < hold_time) {  // hold time
    if (last_y_delta > 0) {
      out_state->gamepad.thumb_ry = SHRT_MIN;
    } else {
      out_state->gamepad.thumb_ry = SHRT_MAX;
    }
  }

  // Return true if either X or Y delta is non-zero or if within the hold time
  if (input_state.mouse.x_delta == 0 && input_state.mouse.y_delta == 0 &&
      elapsed_x >= hold_time && elapsed_y >= hold_time) {
    return false;
  }

  XThread* current_thread = XThread::GetCurrentThread();

  if (!current_thread) {
    return false;
  }

  xe::be<float>* addition_x = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].x_address);

  xe::be<float>* radian_y = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].y_address);


  float degree_x = *addition_x;
  float degree_y = RadianstoDegree(*radian_y);

  // X-axis = 0 to 360
  if (!cvars::invert_x) {
    degree_x += (input_state.mouse.x_delta / 5.f) * (float)cvars::sensitivity;
  } else {
    degree_x -= (input_state.mouse.x_delta / 5.f) * (float)cvars::sensitivity;
  }

  *addition_x = degree_x;

    if (!cvars::invert_y) {
      degree_y += (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
    } else {
      degree_y -= (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
    }

    *radian_y = DegreetoRadians(degree_y);
  return true;
}
std::string SaintsRow1Game::ChooseBinds() { return "Default"; }
bool SaintsRow1Game::ModifierKeyHandler(uint32_t user_index,
                                       RawInputState& input_state,
                                       X_INPUT_STATE* out_state) {
  return false;
}
}  // namespace winkey
}  // namespace hid
}  // namespace xe