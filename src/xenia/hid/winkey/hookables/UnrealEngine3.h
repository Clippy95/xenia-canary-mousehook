/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_WINKEY_UnrealEngine3_H_
#define XENIA_HID_WINKEY_UnrealEngine3_H_

#include "xenia/hid/winkey/hookables/hookable_game.h"

namespace xe {
namespace hid {
namespace winkey {

class GearsOfWarsGame : public HookableGame {
 public:
  enum class GameBuild {
    Unknown,
    GearsOfWars2_TU6,
    GearsOfWars1_TU0,
    GearsOfWars3_TU0,
    GearsOfWars3_TU6,
    GearsOfWarsJudgment_TU0,
    GearsOfWarsJudgment_TU4,
    ArmyOfTwo2_TU0,
    ArmyOfTwo1_TU0
  };

  ~GearsOfWarsGame() override;

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

#endif  // XENIA_HID_WINKEY_UnrealEngine3_H_