/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/CallOfDuty.h"

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

const uint32_t kTitleIdCOD4 = 0x415607E6;
const uint32_t kTitleIdCOD3 = 0x415607E1;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  uint32_t check_addr;
  uint32_t check_value;
  uint32_t title_id;
  uint32_t x_address;
  uint32_t y_address;
};

std::map<CallOfDutyGame::GameBuild, GameBuildAddrs> supported_builds{
    {CallOfDutyGame::GameBuild::Unknown, {NULL, NULL, NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_253SP,
     {0x8204EB24, 0x63675F66, kTitleIdCOD4, 0x8261246C, 0x82612468}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_253MP,
     {0x82055EF4, 0x63675F66, kTitleIdCOD4, 0x82B859B8, 0x82B859B4}},
    {CallOfDutyGame::GameBuild::CallOfDuty3_SP,
     {0x82078F00, 0x63675F66, kTitleIdCOD3, 0x82A58F68, 0x82A58F64}}};

CallOfDutyGame::~CallOfDutyGame() = default;

bool CallOfDutyGame::IsGameSupported() {
  auto title_id = kernel_state()->title_id();

  if (title_id != kTitleIdCOD4 && title_id != kTitleIdCOD3) {
    return false;
  }

  for (auto& build : supported_builds) {
    auto* build_ptr = kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
        build.second.check_addr);

    if (*build_ptr == build.second.check_value) {
      game_build_ = build.first;
      return true;
    }
  }

  return false;
}

bool CallOfDutyGame::DoHooks(uint32_t user_index, RawInputState& input_state,
                             X_INPUT_STATE* out_state) {
  if (!IsGameSupported()) {
    return false;
  }

  // if (supported_builds.count(game_build_) == 0) {
  //   return false;
  // }

  // Don't constantly write if there is no mouse movement.

  XThread* current_thread = XThread::GetCurrentThread();

  if (!current_thread) {
    return false;
  }

  xe::be<float>* degree_x = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].x_address);

  xe::be<float>* degree_y = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].y_address);

  float new_degree_x = *degree_x;
  float new_degree_y = *degree_y;
  /* if (!radian_x || *radian_x == NULL) {
    // Not in game
    return false;
  }
  */

  // X-axis = 0 to 360
  if (!cvars::invert_x) {
    new_degree_x -=
        (input_state.mouse.x_delta / 5.f) * (float)cvars::sensitivity;
  } else {
    new_degree_x +=
        (input_state.mouse.x_delta / 5.f) * (float)cvars::sensitivity;
  }
  *degree_x = new_degree_x;

  if (!cvars::invert_y) {
    new_degree_y +=
        (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
  } else {
    new_degree_y -=
        (input_state.mouse.y_delta / 5.f) * (float)cvars::sensitivity;
  }
  *degree_y = new_degree_y;

  return true;
}

std::string CallOfDutyGame::ChooseBinds() { return "Default"; }

bool CallOfDutyGame::ModifierKeyHandler(uint32_t user_index,
                                        RawInputState& input_state,
                                        X_INPUT_STATE* out_state) {
  return false;
}
}  // namespace winkey
}  // namespace hid
}  // namespace xe