/*
 *  Light Gun Testing Core
 *
 *  Copyright (C) 2019 beardypig <beardypig@protonmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <cmath>

#include "libretro.h"

// Callbacks
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

static const size_t FRAME_WIDTH = 320;
static const size_t FRAME_HEIGHT = 240;
uint16_t video_buffer[FRAME_WIDTH * FRAME_HEIGHT];
static const uint16_t FOREGROUND_COLOR = 0x07ff;
static const uint16_t BACKGROUND_COLOR = 0x528a;

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

// Cheats
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

// Load a cartridge
bool retro_load_game(const struct retro_game_info *info) {
  return true;
}

bool retro_load_game_special(unsigned game_type,
                             const struct retro_game_info *info,
                             size_t num_info) {
  return false;
}

// Unload the cartridge
void retro_unload_game(void) {}

unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }

// libretro unused api functions
void retro_set_controller_port_device(unsigned port, unsigned device) {}

void *retro_get_memory_data(unsigned id) { return nullptr; }
size_t retro_get_memory_size(unsigned id) { return 0; }

// Serialisation methods
size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void *data, size_t size) { return false; }
bool retro_unserialize(const void *data, size_t size) { return false; }

// End of retrolib
void retro_deinit(void) {}

// libretro global setters
void retro_set_environment(retro_environment_t cb) {
  environ_cb = cb;
  bool no_rom = true;
  cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {}
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_init(void) {
  /* set up some logging */
  struct retro_log_callback log;
  unsigned level = 4;

  if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
    log_cb = log.log;
  else
    log_cb = nullptr;

  // the performance level is guide to frontend to give an idea of how intensive
  // this core is to run
  environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

/*
 * Tell libretro about this core, it's name, version and which rom files it
 * supports.
 */
void retro_get_system_info(struct retro_system_info *info) {
  memset(info, 0, sizeof(*info));
  info->library_name = "Lightgun Testing";
  info->library_version = "1.0.0";
  info->need_fullpath = false;
  info->valid_extensions = "";
}

/*
 * Tell libretro about the AV system; the fps, sound sample rate and the
 * resolution of the display.
 */
void retro_get_system_av_info(struct retro_system_av_info *info) {
  int pixel_format = RETRO_PIXEL_FORMAT_RGB565;

  memset(info, 0, sizeof(retro_system_av_info));
  info->timing.fps = static_cast<double>(60);
  info->timing.sample_rate = static_cast<double>(44100);
  info->geometry.base_width = FRAME_WIDTH;
  info->geometry.base_height = FRAME_HEIGHT;
  info->geometry.max_width = FRAME_WIDTH;
  info->geometry.max_height = FRAME_HEIGHT;

  environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

void retro_reset(void) {
}

void get_lightgun_position(unsigned port, int &x, int &y, int16_t &offscreen, int16_t &offscreen_shot, int16_t &trigger)
{
  offscreen = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN);
  offscreen_shot = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
  trigger = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER);

  x = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X);
  y = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y);
}


void draw_crosshair(int x, int y, u_int16_t color) {
  for (int i = 3; i < 10; i++) {
    if ((x + i) < FRAME_WIDTH) {
      video_buffer[(y * FRAME_WIDTH) + (x + i)] = color;
    }
    if ((x - i) >= 0) {
      video_buffer[(y * FRAME_WIDTH) + (x - i)] = color;
    }

    if ((y + i) < FRAME_HEIGHT) {
      video_buffer[((y + i) * FRAME_WIDTH) + x] = color;
    }
    if ((y - i) >= 0) {
      video_buffer[((y - i) * FRAME_WIDTH) + x] = color;
    }
  }
}

// Run a single frame of emulation
void retro_run(void) {
  int x, y;
  int16_t offscreen, offscreen_shot, trigger;

  for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; i++)
    video_buffer[i] = BACKGROUND_COLOR;

  get_lightgun_position(0, x, y, offscreen, offscreen_shot, trigger);

  if (!offscreen) {
    draw_crosshair(x, y, FOREGROUND_COLOR);
  }

  video_cb(video_buffer, FRAME_WIDTH, FRAME_HEIGHT, sizeof(uint16_t) * FRAME_WIDTH);
}
