/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#define _USE_MATH_DEFINES

#include "xenia/hid/winkey/hookables/GTAV.h"

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

const uint32_t kTitleIdGTAV = 0x545408A7;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  const char* title_version;
  uint32_t base_address;
  uint32_t x_offset;
  uint32_t z_offset;
  uint32_t y_offset;
};

std::map<GTAVGame::GameBuild, GameBuildAddrs> supported_builds{
    {GTAVGame::GameBuild::GTAV_TU26, {"9.0.26", 0x839FFFF4, 0x2D0,0x2D4 , 0x2D8}}};

GTAVGame::~GTAVGame() = default;

bool GTAVGame::IsGameSupported() {
  if (kernel_state()->title_id() != kTitleIdGTAV) {
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

float GTAVGame::DegreetoRadians(float degree) {
  return (float)(degree * (M_PI / 180));
}

float GTAVGame::RadianstoDegree(float radians) {
  return (float)(radians * (180 / M_PI));
}

bool GTAVGame::DoHooks(uint32_t user_index, RawInputState& input_state,
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

    xe::be<uint32_t>* base_address =
      kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
          supported_builds[game_build_].base_address);

  if (!base_address || *base_address == NULL) {
    // Not in game
    return false;
  }

  xe::be<uint32_t> x_address =
      *base_address + supported_builds[game_build_].x_offset;
  xe::be<uint32_t> y_address =
      *base_address + supported_builds[game_build_].y_offset;
  xe::be<uint32_t> z_address =
      *base_address + supported_builds[game_build_].z_offset;

  xe::be<float>* degree_x_act =
      kernel_memory()->TranslateVirtual<xe::be<float>*>(x_address);
  xe::be<float>* degree_y_act =
      kernel_memory()->TranslateVirtual<xe::be<float>*>(y_address);
  xe::be<float>* degree_z_act =
      kernel_memory()->TranslateVirtual<xe::be<float>*>(z_address);

  float HorX = *degree_x_act;
  float Vert = *degree_y_act;
  float HorY = *degree_z_act;



 
  Vert += input_state.mouse.y_delta / 50.f;

  //*degree_x_act = RadianstoDegree(radian_HorX);
  *degree_y_act = Vert;
 // *degree_z_act = RadianstoDegree(radian_HorY);

  return true;
}

std::string GTAVGame::ChooseBinds() { return "Default"; }

bool GTAVGame::ModifierKeyHandler(uint32_t user_index,
                                        RawInputState& input_state,
                                        X_INPUT_STATE* out_state) {
  return false;
}
}  // namespace winkey
}  // namespace hid
}  // namespace xe