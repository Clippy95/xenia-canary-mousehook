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
  const char* title_version;
  uint32_t x_address;
  uint32_t y_address;
  uint32_t vehicle_address;
  uint32_t weapon_wheel_address;
  uint32_t menu_status_address;
  uint32_t currentFPS_address;
  uint32_t havok_frametime_address;
};

std::map<SaintsRow1Game::GameBuild, GameBuildAddrs> supported_builds{
    {SaintsRow1Game::GameBuild::Unknown, {" ", NULL, NULL}},
    {SaintsRow1Game::GameBuild::SaintsRow1_TU1,
     {"1.0.1", 0x827f9af8, 0x827F9B00, 0x827F9A6A, 0x8283CA7B, 0x835F27A3,
      0x827CA750, 0x835F2684}}};

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
      ResetCamXBasedOnFPS();
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
  xe::be<float>* currentFPS = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].currentFPS_address);
  xe::be<float>* frametime = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].havok_frametime_address);
  float correctFrametime;
  if (*currentFPS <= 60.f)
    correctFrametime = *currentFPS / 60.f;
  else
    correctFrametime = 1.f;

  //*frametime = correctFrametime * 2;

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
  // no accumluation as it's expected every frame?
  camx += (float)input_state.mouse.x_delta;
  if (!cvars::invert_x) {
    degree_x = ((camx / 5.f) * (float)cvars::sensitivity) * correctFrametime;
  } else {
    degree_x = ((camx / 5.f) * (float)cvars::sensitivity) * correctFrametime;
  }

  *addition_x = degree_x;

  if (!cvars::invert_y) {
    degree_y += (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
  } else {
    degree_y -= (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
  }

  ResetCamXBasedOnFPS();

  *radian_y = DegreetoRadians(degree_y);
  return true;
}
std::string SaintsRow1Game::ChooseBinds() {
  auto* wheel_status = kernel_memory()->TranslateVirtual<uint8_t*>(
      supported_builds[game_build_].weapon_wheel_address);
  auto* menu_status = kernel_memory()->TranslateVirtual<uint8_t*>(
      supported_builds[game_build_].menu_status_address);
  auto* vehicle_status = kernel_memory()->TranslateVirtual<uint8_t*>(
      supported_builds[game_build_].vehicle_address);

  if (*wheel_status == 1) {
    return "Default";
  }
  if (vehicle_status && *vehicle_status != 0) {
    return "Vehicle";
  }

  return "Default";
}
bool SaintsRow1Game::ModifierKeyHandler(uint32_t user_index,
                                        RawInputState& input_state,
                                        X_INPUT_STATE* out_state) {
  float thumb_lx = (int16_t)out_state->gamepad.thumb_lx;
  float thumb_ly = (int16_t)out_state->gamepad.thumb_ly;

  if (thumb_lx != 0 ||
      thumb_ly !=
          0) {  // Required otherwise stick is pushed to the right by default.
    // Work out angle from the current stick values
    float angle = atan2f(thumb_ly, thumb_lx);

    // Sticks get set to SHRT_MAX if key pressed, use half of that
    float distance = (float)SHRT_MAX;
    distance /= 2;

    out_state->gamepad.thumb_lx = (int16_t)(distance * cosf(angle));
    out_state->gamepad.thumb_ly = (int16_t)(distance * sinf(angle));
  }
  // Return true to signal that we've handled the modifier, so default modifier
  // won't be used
  return true;
}

void SaintsRow1Game::HandleRightStickWorkAround(RawInputState& input_state,
                                                X_INPUT_STATE* out_state) {
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
}
void SaintsRow1Game::ResetCamXBasedOnFPS() {
  static float camx = 0;
  static auto last_time =
      std::chrono::steady_clock::now();  // Track the last time this function
                                         // was called

  // Translate the currentFPS virtual memory address
  xe::be<float>* currentFPS = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].currentFPS_address);

  if (!currentFPS || *currentFPS <= 0) {
    return;  // Don't proceed if currentFPS is invalid or 0
  }

  // Calculate the time in milliseconds for the current frame
  float ms_per_frame = 1000.f / *currentFPS;

  // Get the current time
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<float, std::milli> elapsed_time = now - last_time;

  // If the elapsed time is greater than or equal to ms_per_frame, reset camx
  if (elapsed_time.count() >= ms_per_frame) {
    camx = 0.f;       // Reset camx
    last_time = now;  // Update the last_time to now
  }

}  // namespace winkey
}  // namespace winkey
}  // namespace hid
}  // namespace xe