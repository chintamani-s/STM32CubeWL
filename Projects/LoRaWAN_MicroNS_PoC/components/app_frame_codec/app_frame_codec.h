#ifndef APP_FRAME_CODEC_H
#define APP_FRAME_CODEC_H

#include <stddef.h>
#include <stdint.h>

#define APP_FRAME_CODEC_WIRE_SIZE 6U

typedef struct
{
  int16_t temperature_c_x100;
  uint8_t button_state;
  uint8_t battery_percent;
  uint16_t status_flags;
} app_frame_t;

typedef enum
{
  APP_FRAME_CODEC_OK = 0,
  APP_FRAME_CODEC_INVALID_ARGUMENT,
  APP_FRAME_CODEC_INVALID_LENGTH,
  APP_FRAME_CODEC_INVALID_FIELD,
  APP_FRAME_CODEC_BUFFER_TOO_SMALL
} app_frame_codec_status_t;

app_frame_codec_status_t app_frame_codec_encode(const app_frame_t *frame,
                                                uint8_t *buffer,
                                                size_t buffer_capacity,
                                                size_t *encoded_length);

app_frame_codec_status_t app_frame_codec_decode(const uint8_t *buffer,
                                                size_t buffer_length,
                                                app_frame_t *frame);

#endif