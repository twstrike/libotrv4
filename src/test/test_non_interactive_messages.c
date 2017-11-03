#include "../constants.h"
#include "../dake.h"
#include "../str.h"

#define PREKEY_BEFORE_PROFILE_BYTES 2 + 1 + 4 + 4

void test_dake_prekey_message_serializes(prekey_message_fixture_t *f,
                                         gconstpointer data) {
  OTR4_INIT;

  ecdh_keypair_t ecdh[1];
  dh_keypair_t dh;

  uint8_t sym[ED448_PRIVATE_BYTES] = {0};
  ecdh_keypair_generate(ecdh, sym);
  otrv4_assert(dh_keypair_generate(dh) == OTR4_SUCCESS);

  dake_prekey_message_t *prekey_message = dake_prekey_message_new(f->profile);
  prekey_message->sender_instance_tag = 1;
  ec_point_copy(prekey_message->Y, ecdh->pub);
  prekey_message->B = dh_mpi_copy(dh->pub);

  uint8_t *serialized = NULL;
  otrv4_assert(dake_prekey_message_asprintf(&serialized, NULL,
                                            prekey_message) == OTR4_SUCCESS);

  char expected[] = {
      0x0,
      0x04,                 // version
      OTR_PRE_KEY_MSG_TYPE, // message type
      0x0,
      0x0,
      0x0,
      0x1, // sender instance tag
      0x0,
      0x0,
      0x0,
      0x0, // receiver instance tag
  };

  uint8_t *cursor = serialized;
  otrv4_assert_cmpmem(cursor, expected, 11); // sizeof(expected));
  cursor += 11;

  size_t user_profile_len = 0;
  uint8_t *user_profile_serialized = NULL;
  otrv4_assert(user_profile_asprintf(&user_profile_serialized,
                                     &user_profile_len,
                                     prekey_message->profile) == OTR4_SUCCESS);
  otrv4_assert_cmpmem(cursor, user_profile_serialized, user_profile_len);
  free(user_profile_serialized);
  cursor += user_profile_len;

  uint8_t serialized_y[ED448_POINT_BYTES + 2] = {0};
  ec_point_serialize(serialized_y, prekey_message->Y);
  otrv4_assert_cmpmem(cursor, serialized_y, sizeof(ec_public_key_t));
  cursor += sizeof(ec_public_key_t);

  uint8_t serialized_b[DH3072_MOD_LEN_BYTES] = {0};
  size_t mpi_len = 0;
  otr4_err_t err = dh_mpi_serialize(serialized_b, DH3072_MOD_LEN_BYTES,
                                    &mpi_len, prekey_message->B);
  otrv4_assert(!err);
  // Skip first 4 because they are the size (mpi_len)
  otrv4_assert_cmpmem(cursor + 4, serialized_b, mpi_len);

  dh_keypair_destroy(dh);
  ecdh_keypair_destroy(ecdh);
  dake_prekey_message_free(prekey_message);
  free(serialized);
  dh_free();
}

void test_dake_prekey_message_deserializes(prekey_message_fixture_t *f,
                                           gconstpointer data) {
  OTR4_INIT;

  ecdh_keypair_t ecdh[1];
  dh_keypair_t dh;

  uint8_t sym[ED448_PRIVATE_BYTES] = {1};
  ecdh_keypair_generate(ecdh, sym);
  otrv4_assert(dh_keypair_generate(dh) == OTR4_SUCCESS);

  dake_prekey_message_t *prekey_message = dake_prekey_message_new(f->profile);
  ec_point_copy(prekey_message->Y, ecdh->pub);
  prekey_message->B = dh_mpi_copy(dh->pub);

  size_t serialized_len = 0;
  uint8_t *serialized = NULL;
  otrv4_assert(dake_prekey_message_asprintf(&serialized, &serialized_len,
                                            prekey_message) == OTR4_SUCCESS);

  dake_prekey_message_t *deserialized = malloc(sizeof(dake_prekey_message_t));
  memset(deserialized, 0, sizeof(dake_prekey_message_t));
  otrv4_assert(dake_prekey_message_deserialize(deserialized, serialized,
                                               serialized_len) == OTR4_SUCCESS);

  // assert prekey eq
  g_assert_cmpuint(deserialized->sender_instance_tag, ==,
                   prekey_message->sender_instance_tag);
  g_assert_cmpuint(deserialized->receiver_instance_tag, ==,
                   prekey_message->receiver_instance_tag);
  otrv4_assert_user_profile_eq(deserialized->profile, prekey_message->profile);
  otrv4_assert_ec_public_key_eq(deserialized->Y, prekey_message->Y);
  otrv4_assert_dh_public_key_eq(deserialized->B, prekey_message->B);

  dh_keypair_destroy(dh);
  ecdh_keypair_destroy(ecdh);
  dake_prekey_message_free(prekey_message);
  dake_prekey_message_free(deserialized);
  free(serialized);

  OTR4_FREE;
}

void test_dake_prekey_message_valid(prekey_message_fixture_t *f,
                                    gconstpointer data) {
  OTR4_INIT;

  ecdh_keypair_t ecdh[1];
  dh_keypair_t dh;

  uint8_t sym[ED448_PRIVATE_BYTES] = {1};
  ecdh_keypair_generate(ecdh, sym);
  otrv4_assert(dh_keypair_generate(dh) == OTR4_SUCCESS);

  dake_prekey_message_t *prekey_message = dake_prekey_message_new(f->profile);
  otrv4_assert(prekey_message != NULL);

  ec_point_copy(prekey_message->Y, ecdh->pub);
  prekey_message->B = dh_mpi_copy(dh->pub);

  otrv4_assert(valid_received_values(prekey_message->Y, prekey_message->B,
                                     prekey_message->profile));

  ecdh_keypair_destroy(ecdh);
  dh_keypair_destroy(dh);
  dake_prekey_message_free(prekey_message);

  ecdh_keypair_t invalid_ecdh[1];
  dh_keypair_t invalid_dh;

  uint8_t invalid_sym[ED448_PRIVATE_BYTES] = {1};
  ecdh_keypair_generate(invalid_ecdh, invalid_sym);
  otrv4_assert(dh_keypair_generate(invalid_dh) == OTR4_SUCCESS);

  user_profile_t *invalid_profile = user_profile_new("2");
  otrv4_shared_prekey_generate(invalid_profile->shared_prekey, sym);
  ec_point_copy(invalid_profile->pub_key, invalid_ecdh->pub);
  dake_prekey_message_t *invalid_prekey_message =
      dake_prekey_message_new(invalid_profile);

  ec_point_copy(invalid_prekey_message->Y, invalid_ecdh->pub);
  invalid_prekey_message->B = dh_mpi_copy(invalid_dh->pub);

  otrv4_assert(!valid_received_values(invalid_prekey_message->Y,
                                      invalid_prekey_message->B,
                                      invalid_prekey_message->profile));

  user_profile_free(invalid_profile);
  ecdh_keypair_destroy(invalid_ecdh);
  dh_keypair_destroy(invalid_dh);
  dake_prekey_message_free(invalid_prekey_message);

  OTR4_FREE;
}