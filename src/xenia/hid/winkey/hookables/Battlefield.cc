/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/Battlefield.h"

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

const uint32_t kTitleIdBattlefield = 0x454107F9;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  const char* title_version;
  uint32_t base_address;
  uint32_t base_offset_1;  // Game has 11 offsets/pointers source:
                           // https://github.com/isJuhn/KAMI/blob/master/KAMI.Core/Games/BattlefieldBadCompany.cs
                           // , this doesn't work right now, need to find these
                           // same pointers.
  uint32_t x_offset;
  uint32_t y_offset;
};

std::map<BattlefieldGame::GameBuild, GameBuildAddrs> supported_builds{
    {BattlefieldGame::GameBuild::Battlefield_TU0,
     {"8.0", 0x1FFD683C, 0x8, 0x8, 0xC}}};

BattlefieldGame::~BattlefieldGame() = default;

bool BattlefieldGame::IsGameSupported() {
  if (kernel_state()->title_id() != kTitleIdBattlefield) {
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

float BattlefieldGame::DegreetoRadians(float degree) {
  return (float)(degree * (M_PI / 180));
}

float BattlefieldGame::RadianstoDegree(float radians) {
  return (float)(radians * (180 / M_PI));
}

bool BattlefieldGame::DoHooks(uint32_t user_index, RawInputState& input_state,
                              X_INPUT_STATE* out_state) {
  if (!IsGameSupported()) {
    return false;
  }

  if (supported_builds.count(game_build_) == 0) {
    return false;
  }

  // Don't constantly write if there is no mouse movement.
  if (input_state.mouse.x_delta == 0 || input_state.mouse.y_delta == 0) {
    return false;
  }

  XThread* current_thread = XThread::GetCurrentThread();

  if (!current_thread) {
    return false;
  }

  // Retrieve the base address and offset
  xe::be<uint32_t>* base_address_ptr =
      kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
          supported_builds[game_build_].base_address);

  if (!base_address_ptr || *base_address_ptr == NULL) {
    // Not in game
    return false;
  }

  // Calculate camera_base by adding base_address and base_offset_1
  uint32_t camera_base =
      *base_address_ptr + supported_builds[game_build_].base_offset_1;

  // Calculate the X and Y addresses using the camera_base
  xe::be<uint32_t> x_address =
      camera_base + supported_builds[game_build_].x_offset;
  xe::be<uint32_t> y_address =
      camera_base + supported_builds[game_build_].y_offset;

  // Translate the virtual addresses for radian values
  xe::be<float>* radian_x =
      kernel_memory()->TranslateVirtual<xe::be<float>*>(x_address);

  xe::be<float>* radian_y =
      kernel_memory()->TranslateVirtual<xe::be<float>*>(y_address);

  if (!radian_x || !radian_y) {
    return false;
  }

  float degree_x = RadianstoDegree(*radian_x);
  float degree_y = RadianstoDegree(*radian_y);

  // X-axis = 0 to 360
  if (!cvars::invert_x) {
    degree_x += (input_state.mouse.x_delta / 50.f) * (float)cvars::sensitivity;
  } else {
    degree_x -= (input_state.mouse.x_delta / 50.f) * (float)cvars::sensitivity;
  }
  *radian_x = DegreetoRadians(degree_x);

  // Y-axis = -90 to 90
  if (!cvars::invert_y) {
    degree_y += (input_state.mouse.y_delta / 50.f) * (float)cvars::sensitivity;
  } else {
    degree_y -= (input_state.mouse.y_delta / 50.f) * (float)cvars::sensitivity;
  }
  *radian_y = DegreetoRadians(degree_y);

  return true;
}

std::string BattlefieldGame::ChooseBinds() { return "Default"; }

bool BattlefieldGame::ModifierKeyHandler(uint32_t user_index,
                                         RawInputState& input_state,
                                         X_INPUT_STATE* out_state) {
  return false;
}
}  // namespace winkey
}  // namespace hid
}  // namespace xe