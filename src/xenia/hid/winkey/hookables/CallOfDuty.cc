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

const uint32_t kTitleIdCODBO2 = 0x415608C3;
const uint32_t kTitleIdCODMW2 = 0x41560817;
const uint32_t kTitleIdCOD4 = 0x415607E6;
const uint32_t kTitleIdCOD3 = 0x415607E1;

namespace xe {
namespace hid {
namespace winkey {
struct GameBuildAddrs {
  uint32_t cg_fov_address;
  uint32_t cg_fov;  // cg_fo
  uint32_t title_id;
  uint32_t x_address;
  uint32_t y_address;
  uint32_t fovscale_address;
  uint32_t base_address;  // Static addresses in older cods, needs pointers in newer cods?
  uint32_t x_offset;
};

std::map<CallOfDutyGame::GameBuild, GameBuildAddrs> supported_builds{
    {CallOfDutyGame::GameBuild::Unknown, {NULL, NULL, NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_253SP,
     {0x8204EB24, 0x63675F66, kTitleIdCOD4, 0x8261246C, 0x82612468,
      0x82612458,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_253MP,
     {0x82055EF4, 0x63675F66, kTitleIdCOD4, 0x82B859B8, 0x82B859B4,
      0x8254EE50,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_270SP,
     {0x8204E7FC, 0x63675F66, kTitleIdCOD4, 0x8262E168, 0x8262E164,
      0x82612458,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_270MP,
     {0x8205617C, 0x63675F66, kTitleIdCOD4, 0x82B9F664, 0x82B9F660,
      0x82558944,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_290SP,
     {0x8203ABE8, 0x63675F66, kTitleIdCOD4, 0x8247C808, 0x8247C804,
      0x82348900,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_290MP,
     {0x82042588, 0x63675F66, kTitleIdCOD4, 0x82A7F57C, 0x82A7F578,
      0x823A1F04,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_328SP,
     {0x82009C80, 0x63675F66, kTitleIdCOD4, 0x826A8640, 0x826A863C,
      0x82567E8C,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty4_Alpha_328MP,
     {0x8200BB2C, 0x63675F66, kTitleIdCOD4, 0xB384B650, 0xB384B64C,
      0x826027D0,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDutyMW2_Alpha_482SP,
     {0x82007560, 0x63675F66, kTitleIdCODMW2, 0x82627D08, 0x82627D04,
      0x824609CC,NULL}},
    {CallOfDutyGame::GameBuild::CallOfDuty3_SP,
     {0x82078F00, 0x63675F66, kTitleIdCOD3, 0x82A58F68, 0x82A58F64,
      0x825CE5F8,NULL}},
    {CallOfDutyGame::GameBuild::New_Moon_PatchedXEX,
     {0x82004860, 0x63675F66, kTitleIdCODBO2, NULL, NULL, 0x82866DAC,
      0x829FA9C8, 0x2C38}}};

CallOfDutyGame::~CallOfDutyGame() = default;

bool CallOfDutyGame::IsGameSupported() {
  auto title_id = kernel_state()->title_id();

  if (title_id != kTitleIdCOD4 && title_id != kTitleIdCOD3 &&
      title_id != kTitleIdCODBO2 && title_id != kTitleIdCODMW2) {
    return false;
  }

  for (auto& build : supported_builds) {
    auto* build_ptr = kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
        build.second.cg_fov_address);

    if (*build_ptr == build.second.cg_fov) {
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

 xe::be<float>* degree_x;
  xe::be<float>* degree_y;

  if (supported_builds[game_build_].base_address != NULL) {
    // Calculate based on base address
    uint32_t base_address =
        *kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(
            supported_builds[game_build_].base_address);
    int32_t offset = supported_builds[game_build_].x_offset;
    degree_x = kernel_memory()->TranslateVirtual<xe::be<float>*>(base_address +
                                                                 offset);
    degree_y = kernel_memory()->TranslateVirtual<xe::be<float>*>(base_address +
                                                                 offset - 4);
  } else {
    // Use pre-defined addresses for other builds
    degree_x = kernel_memory()->TranslateVirtual<xe::be<float>*>(
        supported_builds[game_build_].x_address);
    degree_y = kernel_memory()->TranslateVirtual<xe::be<float>*>(
        supported_builds[game_build_].y_address);
  }
  xe::be<float>* fovscale = kernel_memory()->TranslateVirtual<xe::be<float>*>(
      supported_builds[game_build_].fovscale_address);

  float new_degree_x = *degree_x;
  float new_degree_y = *degree_y;
  float calc_fovscale = *fovscale;
  /* if (!radian_x || *radian_x == NULL) {
    // Not in game
    return false;
  }
  */

  if (calc_fovscale == 0) {  // Required check otherwise mouse stops working.
    calc_fovscale = 1.f;
  }
  const float a =
      0.1f;  // Quadratic scaling to make fovscale effect sens stronger
  if (calc_fovscale != 1.f) {
    calc_fovscale =
        (1 - a) * (calc_fovscale * calc_fovscale) + a * calc_fovscale;
  }

  float divsor;
  divsor = 7.5f / calc_fovscale;

  // X-axis = 0 to 360
  if (!cvars::invert_x) {
    new_degree_x -=
        (input_state.mouse.x_delta / divsor) * (float)cvars::sensitivity;
  } else {
    new_degree_x +=
        (input_state.mouse.x_delta / divsor) * (float)cvars::sensitivity;
  }
  *degree_x = new_degree_x;

  if (!cvars::invert_y) {
    new_degree_y +=
        (input_state.mouse.y_delta / divsor) * (float)cvars::sensitivity;
  } else {
    new_degree_y -=
        (input_state.mouse.y_delta / divsor) * (float)cvars::sensitivity;
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