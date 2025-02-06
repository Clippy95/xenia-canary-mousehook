/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/GTAIV.h"

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
#define DTOR 0.01745329251
const uint32_t kTitleIdGTAIV = 0x545407F2;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  const char* title_version;
  uint32_t cameracontroller_pointer_address;
  uint32_t SteerAddYaw_offset;
  uint32_t SteerAddPitch_offset;
};

std::map<GTAIVGame::GameBuild, GameBuildAddrs> supported_builds{
    {GTAIVGame::GameBuild::GTAIV_TU0, {"5.0", 0x46965100, 0x130, 0x12C}}};

GTAIVGame::~GTAIVGame() = default;

bool GTAIVGame::IsGameSupported() {
  if (kernel_state()->title_id() != kTitleIdGTAIV) {
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

bool GTAIVGame::DoHooks(uint32_t user_index, RawInputState& input_state,
                        X_INPUT_STATE* out_state) {
  if (!IsGameSupported()) {
    return false;
  }

  if (supported_builds.count(game_build_) == 0) {
    return false;
  }

  XThread* current_thread = XThread::GetCurrentThread();

  if (!current_thread) {
    return false;
  }

  xe::be<double>* add_x =
      kernel_memory()->TranslateVirtual<xe::be<double>*>(0x91C0ECD8);

  xe::be<double>* add_y =
      kernel_memory()->TranslateVirtual<xe::be<double>*>(0x91C0ECE0);

  xe::be<uint8_t>* center_flag =
      kernel_memory()->TranslateVirtual<xe::be<uint8_t>*>(0x91C0ECD4);
  HandleCenterFlag(center_flag, input_state);
  double camx = *add_x;
  double camy = *add_y;
  // X-axis = 0 to 360

  camx =
      ((-input_state.mouse.x_delta / 5.0) * (double)cvars::sensitivity) * DTOR;

  camy =
      ((-input_state.mouse.y_delta / 5.0) * (double)cvars::sensitivity) * DTOR;

  *add_x = camx;

  *add_y = camy;

  return true;
}

void GTAIVGame::HandleCenterFlag(xe::be<uint8_t>* center_flag,
                                 const RawInputState& input_state) {
  if (!center_flag) {
    return;
  }

  // Detect mouse movement
  if (input_state.mouse.x_delta != 0 || input_state.mouse.y_delta != 0) {
    last_movement_time = std::chrono::steady_clock::now();

    *center_flag = 1;
  }

  // Check if 5 seconds have passed since the last movement
  if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - last_movement_time)
          .count() >= 1250) {
    *center_flag = 0;
  }
}

std::string GTAIVGame::ChooseBinds() { return "Default"; }

bool GTAIVGame::ModifierKeyHandler(uint32_t user_index,
                                   RawInputState& input_state,
                                   X_INPUT_STATE* out_state) {
  return false;
}

void GTAIVGame::WeaponSwitchHandler(uint32_t user_index,
                                    RawInputState& input_state,
                                    X_INPUT_STATE* out_state, int weapon,
                                    uint16_t buttons) {}

}  // namespace winkey
}  // namespace hid
}  // namespace xe