#include "app_frame_codec.h"

static app_frame_codec_status_t validate_frame(const app_frame_t *frame)
{
  if ((frame->button_state > 1U) || (frame->battery_percent > 100U))
  {
    return APP_FRAME_CODEC_INVALID_FIELD;
  }

  return APP_FRAME_CODEC_OK;
}

app_frame_codec_status_t app_frame_codec_encode(const app_frame_t *frame,
                                                uint8_t *buffer,
                                                size_t buffer_capacity,
                                                size_t *encoded_length)
{
  uint16_t temperature;
  app_frame_codec_status_t status;

  if (encoded_length != NULL)
  {
    *encoded_length = 0U;
  }

  if ((frame == NULL) || (buffer == NULL) || (encoded_length == NULL))
  {
    return APP_FRAME_CODEC_INVALID_ARGUMENT;
  }

  status = validate_frame(frame);
  if (status != APP_FRAME_CODEC_OK)
  {
    return status;
  }

  if (buffer_capacity < APP_FRAME_CODEC_WIRE_SIZE)
  {
    return APP_FRAME_CODEC_BUFFER_TOO_SMALL;
  }

  temperature = (uint16_t) frame->temperature_c_x100;
  buffer[0] = (uint8_t) (temperature >> 8);
  buffer[1] = (uint8_t) temperature;
  buffer[2] = frame->button_state;
  buffer[3] = frame->battery_percent;
  buffer[4] = (uint8_t) (frame->status_flags >> 8);
  buffer[5] = (uint8_t) frame->status_flags;
  *encoded_length = APP_FRAME_CODEC_WIRE_SIZE;

  return APP_FRAME_CODEC_OK;
}

app_frame_codec_status_t app_frame_codec_decode(const uint8_t *buffer,
                                                size_t buffer_length,
                                                app_frame_t *frame)
{
  app_frame_t decoded;
  uint16_t temperature;

  if ((buffer == NULL) || (frame == NULL))
  {
    return APP_FRAME_CODEC_INVALID_ARGUMENT;
  }

  if (buffer_length != APP_FRAME_CODEC_WIRE_SIZE)
  {
    return APP_FRAME_CODEC_INVALID_LENGTH;
  }

  temperature = (uint16_t) (((uint16_t) buffer[0] << 8) | buffer[1]);
  if (temperature <= INT16_MAX)
  {
    decoded.temperature_c_x100 = (int16_t) temperature;
  }
  else
  {
    decoded.temperature_c_x100 = (int16_t) (-1 - (int32_t) (UINT16_MAX - temperature));
  }
  decoded.button_state = buffer[2];
  decoded.battery_percent = buffer[3];
  decoded.status_flags = (uint16_t) (((uint16_t) buffer[4] << 8) | buffer[5]);

  if (validate_frame(&decoded) != APP_FRAME_CODEC_OK)
  {
    return APP_FRAME_CODEC_INVALID_FIELD;
  }

  *frame = decoded;
  return APP_FRAME_CODEC_OK;
}