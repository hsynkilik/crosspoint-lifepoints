#include "LifePointsActivity.h"

#include <Arduino.h>
#include <HalGPIO.h>
#include <algorithm>
#include <utility>

#include "activities/RenderLock.h"

namespace {

constexpr uint8_t BUTTONS[4] = {
    HalGPIO::BTN_BACK,
    HalGPIO::BTN_CONFIRM,
    HalGPIO::BTN_LEFT,
    HalGPIO::BTN_RIGHT
};

constexpr uint8_t EXIT_KEYS[4] = {0, 3, 0, 3};

constexpr uint8_t DIGITS[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

}  // namespace

void LifePointsActivity::onEnter() {
  Activity::onEnter();

  previousOrientation_ = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);

  life_[0] = START_LIFE;
  life_[1] = START_LIFE;

  resetTriggered_ = false;
  exitProgress_ = 0;
  exitStarted_ = 0;

  for (int i = 0; i < 4; ++i) {
    previous_[i] = false;
    pressedAt_[i] = 0;
  }

  requestUpdate();
}

void LifePointsActivity::onExit() {
  renderer.setOrientation(previousOrientation_);
  Activity::onExit();
}

void LifePointsActivity::changeLife(uint8_t player, int amount) {
  const int next = std::max(0, std::min(MAX_LIFE, life_[player] + amount));

  if (next != life_[player]) {
    life_[player] = next;
    requestUpdate();
  }
}

void LifePointsActivity::processExitSequence(uint8_t index, uint32_t now) {
  if (exitProgress_ != 0 && now - exitStarted_ > EXIT_SEQUENCE_MS) {
    exitProgress_ = 0;
  }

  if (index == EXIT_KEYS[exitProgress_]) {
    if (exitProgress_ == 0) {
      exitStarted_ = now;
    }

    ++exitProgress_;

    if (exitProgress_ == 4) {
      exitProgress_ = 0;
      onGoHome();
    }
  } else {
    exitProgress_ = (index == EXIT_KEYS[0]) ? 1 : 0;

    if (exitProgress_ == 1) {
      exitStarted_ = now;
    }
  }
}

void LifePointsActivity::processButton(uint8_t index, bool pressed, uint32_t now) {
  if (pressed && !previous_[index]) {
    pressedAt_[index] = now;
    previous_[index] = true;
    return;
  }

  if (!pressed && previous_[index]) {
    previous_[index] = false;

    const uint32_t duration = now - pressedAt_[index];

    if (index == 1 && resetTriggered_) {
      resetTriggered_ = false;
      return;
    }

    if (duration >= RESET_HOLD_MS && index == 1) {
      return;
    }

    const int step = duration >= LONG_PRESS_MS ? 5 : 1;

    switch (index) {
      case 0:
        changeLife(0, -step);
        break;
      case 1:
        changeLife(0, step);
        break;
      case 2:
        changeLife(1, step);
        break;
      case 3:
        changeLife(1, -step);
        break;
    }

    if (duration < LONG_PRESS_MS) {
      processExitSequence(index, now);
    } else {
      exitProgress_ = 0;
    }
  }
}

void LifePointsActivity::loop() {
  const uint32_t now = millis();

  for (uint8_t i = 0; i < 4; ++i) {
    const bool pressed = mappedInput.isPhysicalFrontButtonPressed(BUTTONS[i]);
    processButton(i, pressed, now);
  }

  if (previous_[1] && !resetTriggered_ &&
      now - pressedAt_[1] >= RESET_HOLD_MS) {
    life_[0] = START_LIFE;
    life_[1] = START_LIFE;
    resetTriggered_ = true;
    exitProgress_ = 0;
    requestUpdate();
  }
}

void LifePointsActivity::drawDigit(int x, int y, int size, int digit, bool clockwise) {
  if (digit < 0 || digit > 9) return;

  const int width = size;
  const int height = size * 2;
  const int thickness = std::max(5, size / 7);

  const uint8_t mask = DIGITS[digit];

  // Draw a rectangle in local digit coordinates, rotated by 90 degrees.
  auto segment = [&](int sx, int sy, int sw, int sh) {
    if (clockwise) {
      renderer.fillRect(x + height - sy - sh, y + sx, sh, sw);
    } else {
      renderer.fillRect(x + sy, y + width - sx - sw, sh, sw);
    }
  };

  if (mask & 0x01) segment(thickness, 0, width - 2 * thickness, thickness);
  if (mask & 0x02) segment(width - thickness, thickness, thickness, height / 2 - thickness);
  if (mask & 0x04) segment(width - thickness, height / 2, thickness, height / 2 - thickness);
  if (mask & 0x08) segment(thickness, height - thickness, width - 2 * thickness, thickness);
  if (mask & 0x10) segment(0, height / 2, thickness, height / 2 - thickness);
  if (mask & 0x20) segment(0, thickness, thickness, height / 2 - thickness);
  if (mask & 0x40) segment(thickness, height / 2 - thickness / 2,
                           width - 2 * thickness, thickness);
}

void LifePointsActivity::drawNumber(int value, int centerX, int centerY, bool clockwise) {
  const int size = 95;
  const int height = size * 2;
  const int spacing = 14;

  int digits[3] = {0, 0, 0};
  int count = 0;

  do {
    digits[count++] = value % 10;
    value /= 10;
  } while (value > 0 && count < 3);

  const int totalWidth = count * size + (count - 1) * spacing;
  const int startX = centerX - height / 2;
  const int startY = centerY - totalWidth / 2;

  for (int i = 0; i < count; ++i) {
    const int digit = clockwise ? digits[count - 1 - i] : digits[i];
    drawDigit(startX, startY + i * (size + spacing),
              size, digit, clockwise);
  }
}

void LifePointsActivity::drawControls() {
  const int w = renderer.getScreenWidth();
  const int h = renderer.getScreenHeight();

  renderer.drawRect(4, 4, w - 8, h - 8);
  renderer.drawLine(8, h / 2, w - 9, h / 2);

  // Simple monochrome indicators.
  renderer.drawRect(16, 16, 44, 44);
  renderer.drawLine(27, 38, 49, 38);

  renderer.drawRect(w - 60, 16, 44, 44);
  renderer.drawLine(w - 49, 38, w - 27, 38);
  renderer.drawLine(w - 38, 27, w - 38, 49);

  renderer.drawRect(16, h - 60, 44, 44);
  renderer.drawLine(27, h - 38, 49, h - 38);
  renderer.drawLine(38, h - 49, 38, h - 27);

  renderer.drawRect(w - 60, h - 60, 44, 44);
  renderer.drawLine(w - 49, h - 38, w - 27, h - 38);
}

void LifePointsActivity::render(RenderLock&& lock) {
  renderer.clearScreen();

  const int w = renderer.getScreenWidth();
  const int h = renderer.getScreenHeight();

  drawControls();
  drawNumber(life_[0], w / 2, h / 4, false);
  drawNumber(life_[1], w / 2, 3 * h / 4, true);
}
