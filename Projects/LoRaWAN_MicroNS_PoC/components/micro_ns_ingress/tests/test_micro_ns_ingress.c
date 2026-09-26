#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "micro_ns_ingress.h"

static void write_relay_metadata(uint8_t *payload, uint8_t channel, int16_t rssi_dbm, int8_t snr_db,
                                 uint8_t data_rate, uint32_t frequency_hz)
{
  uint32_t rssi_code = (uint32_t) (-(rssi_dbm + 15));
  uint32_t snr_code = (uint32_t) (snr_db + 20);
  uint32_t metadata = ((uint32_t) channel << 16) | (rssi_code << 9) | (snr_code << 4) | data_rate;
  uint32_t frequency_step = frequency_hz / 100U;

  payload[0] = (uint8_t) metadata;
  payload[1] = (uint8_t) (metadata >> 8);
  payload[2] = (uint8_t) (metadata >> 16);
  payload[3] = (uint8_t) frequency_step;
  payload[4] = (uint8_t) (frequency_step >> 8);
  payload[5] = (uint8_t) (frequency_step >> 16);
}

static void test_join_request(void)
{
  uint8_t relay_payload[6U + 23U] = { 0U };
  micro_ns_uplink_view_t uplink = { 0 };

  write_relay_metadata(relay_payload, 2U, -87, -3, 3U, 915200000U);
  relay_payload[6] = 0x00U;

  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink) == MICRO_NS_INGRESS_OK);
  assert(uplink.frame_type == MICRO_NS_FRAME_JOIN_REQUEST);
  assert(uplink.relay_channel == 2U);
  assert(uplink.data_rate == 3U);
  assert(uplink.rssi_dbm == -87);
  assert(uplink.snr_db == -3);
  assert(uplink.frequency_hz == 915200000U);
  assert(uplink.phy_payload_length == 23U);
  assert(uplink.mic == &relay_payload[25]);
}

static void test_data_uplink(void)
{
  uint8_t relay_payload[6U + 16U] = { 0U };
  uint8_t *phy_payload = &relay_payload[6];
  micro_ns_uplink_view_t uplink = { 0 };

  write_relay_metadata(relay_payload, 1U, -110, 8, 4U, 904300000U);
  phy_payload[0] = 0x40U;
  phy_payload[1] = 0x26U;
  phy_payload[2] = 0x01U;
  phy_payload[3] = 0x1BU;
  phy_payload[4] = 0xDAU;
  phy_payload[5] = 0x00U;
  phy_payload[6] = 0x34U;
  phy_payload[7] = 0x12U;
  phy_payload[8] = 10U;
  phy_payload[9] = 0xA1U;
  phy_payload[10] = 0xA2U;
  phy_payload[11] = 0xA3U;

  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink) == MICRO_NS_INGRESS_OK);
  assert(uplink.frame_type == MICRO_NS_FRAME_DATA_UPLINK);
  assert(uplink.dev_addr == 0xDA1B0126U);
  assert(uplink.fcnt16 == 0x1234U);
  assert(uplink.fport_present);
  assert(uplink.fport == 10U);
  assert(uplink.frm_payload == &phy_payload[9]);
  assert(uplink.frm_payload_length == 3U);
  assert(uplink.mic == &phy_payload[12]);
}

static void test_rejects_invalid_frames(void)
{
  uint8_t relay_payload[6U + 16U] = { 0U };
  uint8_t *phy_payload = &relay_payload[6];
  micro_ns_uplink_view_t uplink = { 0 };

  write_relay_metadata(relay_payload, 0U, -80, 0, 0U, 915000000U);
  phy_payload[0] = 0x40U;
  phy_payload[5] = 0x0FU;
  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink)
         == MICRO_NS_INGRESS_MALFORMED_FRAME);

  phy_payload[0] = 0x60U;
  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink)
         == MICRO_NS_INGRESS_UNSUPPORTED_FRAME);

  phy_payload[0] = 0x41U;
  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink)
         == MICRO_NS_INGRESS_MALFORMED_FRAME);

  write_relay_metadata(relay_payload, 0U, -80, 0, 0U, 0U);
  phy_payload[0] = 0x40U;
  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink)
         == MICRO_NS_INGRESS_INVALID_METADATA);
}

static void test_rejects_short_lengths_without_modifying_output(void)
{
  const uint8_t relay_payload[6] = { 0U };
  micro_ns_uplink_view_t uplink = { 0 };
  uplink.dev_addr = 0x12345678U;

  assert(micro_ns_parse_relay_uplink(relay_payload, sizeof(relay_payload), &uplink)
         == MICRO_NS_INGRESS_INVALID_LENGTH);
  assert(uplink.dev_addr == 0x12345678U);
}

int main(void)
{
  test_join_request();
  test_data_uplink();
  test_rejects_invalid_frames();
  test_rejects_short_lengths_without_modifying_output();
  return 0;
}