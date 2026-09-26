#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "../app_frame_codec.h"

static void test_round_trip(void)
{
  const app_frame_t input = { -1234, 1U, 87U, 0xA15CU };
  app_frame_t output = { 0 };
  uint8_t buffer[APP_FRAME_CODEC_WIRE_SIZE] = { 0 };
  size_t encoded_length = 0U;

  assert(app_frame_codec_encode(&input, buffer, sizeof(buffer), &encoded_length) == APP_FRAME_CODEC_OK);
  assert(encoded_length == APP_FRAME_CODEC_WIRE_SIZE);
  assert(buffer[0] == 0xFBU);
  assert(buffer[1] == 0x2EU);
  assert(buffer[2] == 1U);
  assert(buffer[3] == 87U);
  assert(buffer[4] == 0xA1U);
  assert(buffer[5] == 0x5CU);
  assert(app_frame_codec_decode(buffer, encoded_length, &output) == APP_FRAME_CODEC_OK);
  assert(output.temperature_c_x100 == input.temperature_c_x100);
  assert(output.button_state == input.button_state);
  assert(output.battery_percent == input.battery_percent);
  assert(output.status_flags == input.status_flags);
}

static void test_extreme_temperatures(void)
{
  const int16_t values[] = { INT16_MIN, -1, 0, INT16_MAX };
  size_t index;

  for (index = 0U; index < sizeof(values) / sizeof(values[0]); index++)
  {
    const app_frame_t input = { values[index], 0U, 0U, 0U };
    app_frame_t output = { 0 };
    uint8_t buffer[APP_FRAME_CODEC_WIRE_SIZE];
    size_t encoded_length = 0U;

    assert(app_frame_codec_encode(&input, buffer, sizeof(buffer), &encoded_length) == APP_FRAME_CODEC_OK);
    assert(app_frame_codec_decode(buffer, encoded_length, &output) == APP_FRAME_CODEC_OK);
    assert(output.temperature_c_x100 == input.temperature_c_x100);
  }
}

static void test_rejects_invalid_payloads(void)
{
  uint8_t payload[APP_FRAME_CODEC_WIRE_SIZE] = { 0U, 0U, 2U, 50U, 0U, 0U };
  app_frame_t output = { 10, 1U, 20U, 30U };

  assert(app_frame_codec_decode(payload, sizeof(payload), &output) == APP_FRAME_CODEC_INVALID_FIELD);
  assert(output.temperature_c_x100 == 10);
  assert(output.button_state == 1U);
  assert(app_frame_codec_decode(payload, sizeof(payload) - 1U, &output) == APP_FRAME_CODEC_INVALID_LENGTH);
  assert(app_frame_codec_decode(payload, sizeof(payload) + 1U, &output) == APP_FRAME_CODEC_INVALID_LENGTH);
}

static void test_rejects_invalid_encode_inputs(void)
{
  app_frame_t input = { 0, 0U, 101U, 0U };
  uint8_t buffer[APP_FRAME_CODEC_WIRE_SIZE] = { 0 };
  size_t encoded_length = 99U;

  assert(app_frame_codec_encode(&input, buffer, sizeof(buffer), &encoded_length) == APP_FRAME_CODEC_INVALID_FIELD);
  assert(encoded_length == 0U);

  input.battery_percent = 100U;
  input.button_state = 2U;
  assert(app_frame_codec_encode(&input, buffer, sizeof(buffer), &encoded_length) == APP_FRAME_CODEC_INVALID_FIELD);

  input.button_state = 0U;
  assert(app_frame_codec_encode(&input, buffer, sizeof(buffer) - 1U, &encoded_length)
         == APP_FRAME_CODEC_BUFFER_TOO_SMALL);
  assert(encoded_length == 0U);
}

int main(void)
{
  test_round_trip();
  test_extreme_temperatures();
  test_rejects_invalid_payloads();
  test_rejects_invalid_encode_inputs();
  return 0;
}