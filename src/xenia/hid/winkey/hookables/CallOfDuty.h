/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_WINKEY_CallOfDuty_H_
#define XENIA_HID_WINKEY_CallOfDuty_H_

#include "xenia/hid/winkey/hookables/hookable_game.h"

namespace xe {
namespace hid {
namespace winkey {

class CallOfDutyGame : public HookableGame {
 public:
  enum class GameBuild {
    Unknown = 0,
    CallOfDuty4_TU0_SP,
    CallOfDuty4_TU0_MP,
    CallOfDuty4_Alpha_253SP,
    CallOfDuty4_Alpha_253MP,
    CallOfDuty4_Alpha_270SP,
    CallOfDuty4_Alpha_270MP,
    CallOfDuty4_Alpha_290SP,
    CallOfDuty4_Alpha_290MP,
    CallOfDuty4_Alpha_328SP,
    CallOfDuty4_Alpha_328MP,
    CallOfDutyMW2_Alpha_482SP,
    CallOfDuty3_SP,
    New_Moon_PatchedXEX
  };

  ~CallOfDutyGame() override;

  bool IsGameSupported();

  bool DoHooks(uint32_t user_index, RawInputState& input_state,
               X_INPUT_STATE* out_state);

  std::string ChooseBinds();

  bool ModifierKeyHandler(uint32_t user_index, RawInputState& input_state,
                          X_INPUT_STATE* out_state);

 private:
  GameBuild game_build_ = GameBuild::Unknown;
};

}  // namespace winkey
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_WINKEY_CallOfDuty_H_
