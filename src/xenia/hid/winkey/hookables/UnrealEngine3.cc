/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/UnrealEngine3.h"

#include "xenia/base/chrono.h"
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

const uint32_t kTitleIdArmyOfTwo2 = 0x454108D8;
const uint32_t kTitleIdArmyOfTwo1 = 0x454107F8;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  const char* title_version;
  uint32_t camera_base_address;
  uint32_t x_offset;
  uint32_t y_offset;
  uint32_t menu_status_address;
};
std::map<ArmyOfTwoGame::GameBuild, GameBuildAddrs> supported_builds{
    {ArmyOfTwoGame::GameBuild::Unknown, {"", NULL, NULL, NULL}},
    {ArmyOfTwoGame::GameBuild::ArmyOfTwo2_TU0,
     {"2.0", 0x7018F6AC, 0x1220, 0x121C, NULL}},
    {ArmyOfTwoGame::GameBuild::ArmyOfTwo1_TU0,
     {"1.0", 0x70188144, 0xE24, 0xE20, 0x830ADFC7}}};

ArmyOfTwoGame::~ArmyOfTwoGame() = default;

bool ArmyOfTwoGame::IsGameSupported() {
  if (kernel_state()->title_id() != kTitleIdArmyOfTwo2 &&
      kernel_state()->title_id() != kTitleIdArmyOfTwo1) {
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

bool ArmyOfTwoGame::DoHooks(uint32_t user_index, RawInputState& input_state,
                            X_INPUT_STATE* out_state) {
  static bool bypass_conditions =
      false;  // This will be set to true 
  static auto start_time = std::chrono::steady_clock::now();
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
  if (supported_builds[game_build_].menu_status_address == NULL) {
    if (!bypass_conditions) {
      auto current_time = std::chrono::steady_clock::now();
      auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
          current_time - start_time);

      if (elapsed_time.count() >= 15) {
        bypass_conditions = true;
      }
    }
  } else {
    auto* menu_status = kernel_memory()->TranslateVirtual<uint8_t*>(
        supported_builds[game_build_].menu_status_address);
    if (menu_status && (*menu_status == 8 || *menu_status == 1)) {
      bypass_conditions = false;
    } else {
      if (!bypass_conditions) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
            current_time - start_time);

        if (elapsed_time.count() >= 2) {
          bypass_conditions = true;
        }
      }
    }
  }
  static uint32_t stored_base_address = 0;
  if (bypass_conditions) {
    uint32_t base_address =
        *kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
            supported_builds[game_build_].camera_base_address);
    if (base_address && base_address >= 0x40000000 &&
        base_address < 0x50000000) {  // timer isn't enough, check location it's
      // most likely between 40000000 - 50000000,
      stored_base_address = base_address;  // Shape shifting address workaround
    }                                      // thanks Marine.

    xe::be<float>* degree_x = kernel_memory()->TranslateVirtual<xe::be<float>*>(
        stored_base_address + supported_builds[game_build_].x_offset);
    xe::be<float>* degree_y = kernel_memory()->TranslateVirtual<xe::be<float>*>(
        stored_base_address + supported_builds[game_build_].y_offset);

    float new_degree_x = *degree_x;
    float new_degree_y = *degree_y;

    if (!cvars::invert_x) {
      new_degree_x += (input_state.mouse.x_delta * 25.1327412287f) *
                      (float)cvars::sensitivity;
    } else {
      new_degree_x -= (input_state.mouse.x_delta * 25.1327412287f) *
                      (float)cvars::sensitivity;
    }
    *degree_x = new_degree_x;

    if (!cvars::invert_y) {
      new_degree_y -= (input_state.mouse.y_delta * 25.1327412287f) *
                      (float)cvars::sensitivity;
    } else {
      new_degree_y += (input_state.mouse.y_delta * 25.1327412287f) *
                      (float)cvars::sensitivity;
    }
    *degree_y = new_degree_y;
  }

  return true;
}

std::string ArmyOfTwoGame::ChooseBinds() { return "Default"; }

bool ArmyOfTwoGame::ModifierKeyHandler(uint32_t user_index,
                                       RawInputState& input_state,
                                       X_INPUT_STATE* out_state) {
  return false;
}
}  // namespace winkey
}  // namespace hid
}  // namespace xe