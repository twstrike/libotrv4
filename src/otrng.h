/*
 *  This file is part of the Off-the-Record Next Generation Messaging
 *  library (libotr-ng).
 *
 *  Copyright (C) 2016-2018, the libotr-ng contributors.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OTRNG_OTRNG_H
#define OTRNG_OTRNG_H

#include "client_profile.h"
#include "client_state.h"
#include "fragment.h"
#include "key_management.h"
#include "keys.h"
#include "prekey_ensemble.h"
#include "prekey_profile.h"
#include "shared.h"
#include "smp.h"
#include "str.h"
#include "v3.h"

#define UNUSED_ARG(x) (void)(x)

#define OTRNG_INIT                                                             \
  do {                                                                         \
    otrng_v3_init();                                                           \
    otrng_dh_init();                                                           \
  } while (0)

#define OTRNG_FREE                                                             \
  do {                                                                         \
    otrng_dh_free();                                                           \
  } while (0)

// TODO: how is this type chosen?
#define POLICY_ALLOW_V3 0x04
#define POLICY_ALLOW_V4 0x05

/* Analogous to v1 and v3 policies */
#define POLICY_NEVER 0x00
#define POLICY_OPPORTUNISTIC (POLICY_ALLOW_V3 | POLICY_ALLOW_V4)
#define POLICY_MANUAL (OTRL_POLICY_ALLOW_V3 | OTRL_POLICY_ALLOW_V4)
#define POLICY_ALWAYS (OTRL_POLICY_ALLOW_V3 | OTRL_POLICY_ALLOW_V4)
#define POLICY_DEFAULT POLICY_OPPORTUNISTIC

typedef struct otrng_s otrng_s; /* Forward declare */

typedef enum {
  OTRNG_STATE_NONE = 0,
  OTRNG_STATE_START = 1,
  OTRNG_STATE_ENCRYPTED_MESSAGES = 2,
  OTRNG_STATE_WAITING_AUTH_I = 3,
  OTRNG_STATE_WAITING_AUTH_R = 4,
  OTRNG_STATE_FINISHED = 5
} otrng_state;

typedef enum {
  OTRNG_ALLOW_NONE = 0,
  OTRNG_ALLOW_V3 = 1,
  OTRNG_ALLOW_V4 = 2
} otrng_supported_version;

typedef enum {
  OTRNG_VERSION_NONE = 0,
  OTRNG_VERSION_3 = 3,
  OTRNG_VERSION_4 = 4
} otrng_version;

// clang-format off
typedef struct otrng_policy_s {
  int allows;
} otrng_policy_s, otrng_policy_p[1];
// clang-format on

// TODO: This is a single instance conversation. Make it multi-instance.
typedef struct otrng_conversation_state_s {
  /* void *opdata; // Could have a conversation opdata to point to a, say
   PurpleConversation */

  otrng_client_state_s *client;
  char *peer;
  uint16_t their_instance_tag;
} otrng_conversation_state_s, otrng_conversation_state_p[1];

struct otrng_s {
  /* Contains: client (private key, instance tag, and callbacks) and
   conversation state */
  otrng_conversation_state_s *conversation;
  otrng_v3_conn_s *v3_conn;

  otrng_state state;
  int supported_versions;

  uint32_t their_prekeys_id;

  uint32_t our_instance_tag;
  uint32_t their_instance_tag;

  client_profile_s *their_client_profile;
  otrng_prekey_profile_s *their_prekey_profile;

  otrng_version running_version;

  key_manager_s *keys;
  smp_context_p smp;

  fragment_context_s *frag_ctx;
}; /* otrng_s */

typedef struct otrng_s otrng_p[1];

// clang-format off
// TODO: this a mock
typedef struct otrng_server_s {
  string_p prekey_message;
} otrng_server_s, otrng_server_p[1];
// clang-format on

typedef enum {
  IN_MSG_NONE = 0,
  IN_MSG_PLAINTEXT = 1,
  IN_MSG_TAGGED_PLAINTEXT = 2,
  IN_MSG_QUERY_STRING = 3,
  IN_MSG_OTR_ENCODED = 4,
  IN_MSG_OTR_ERROR = 5
} otrng_in_message_type;

typedef enum {
  OTRNG_WARN_NONE = 0,
  OTRNG_WARN_RECEIVED_UNENCRYPTED,
  OTRNG_WARN_RECEIVED_NOT_VALID
} otrng_warning;

typedef struct otrng_response_s {
  string_p to_display;
  string_p to_send;
  tlv_list_s *tlvs;
  otrng_warning warning;
} otrng_response_s, otrng_response_p[1];

typedef struct otrng_header_s {
  otrng_supported_version version;
  uint8_t type;
} otrng_header_s, otrng_header_p[1];

INTERNAL otrng_s *otrng_new(struct otrng_client_state_s *state,
                            otrng_policy_s policy);

INTERNAL void otrng_free(/*@only@ */ otrng_s *otr);

INTERNAL otrng_err otrng_build_query_message(string_p *dst,
                                             const string_p message,
                                             const otrng_s *otr);

INTERNAL otrng_response_s *otrng_response_new(void);

INTERNAL void otrng_response_free(otrng_response_s *response);

INTERNAL otrng_err otrng_receive_message(otrng_response_s *response,
                                         const string_p message, otrng_s *otr);

// TODO: this should be called otrng_send_message()
INTERNAL otrng_err otrng_prepare_to_send_message(string_p *to_send,
                                                 const string_p message,
                                                 tlv_list_s **tlvs,
                                                 uint8_t flags, otrng_s *otr);

INTERNAL otrng_err otrng_close(string_p *to_send, otrng_s *otr);

INTERNAL otrng_err otrng_smp_start(string_p *to_send, const uint8_t *question,
                                   const size_t q_len, const uint8_t *secret,
                                   const size_t secretlen, otrng_s *otr);

INTERNAL otrng_err otrng_smp_continue(string_p *to_send, const uint8_t *secret,
                                      const size_t secretlen, otrng_s *otr);

INTERNAL otrng_err otrng_expire_session(string_p *to_send, otrng_s *otr);

API otrng_err otrng_build_whitespace_tag(string_p *whitespace_tag,
                                         const string_p message,
                                         const otrng_s *otr);

API otrng_err otrng_send_symkey_message(string_p *to_send, unsigned int use,
                                        const unsigned char *usedata,
                                        size_t usedatalen, uint8_t *extra_key,
                                        otrng_s *otr);

API otrng_err otrng_smp_abort(string_p *to_send, otrng_s *otr);

API otrng_err otrng_send_offline_message(string_p *dst,
                                         const prekey_ensemble_s *ensemble,
                                         const string_p message, otrng_s *otr);

API otrng_err otrng_send_non_interactive_auth_msg(string_p *dst,
                                                  const string_p message,
                                                  otrng_s *otr);

API otrng_err otrng_heartbeat_checker(string_p *to_send, otrng_s *otr);

API void otrng_v3_init(void);

INTERNAL prekey_ensemble_s *otrng_build_prekey_ensemble(otrng_s *otr);

#ifdef OTRNG_OTRNG_PRIVATE

tstatic void otrng_destroy(otrng_s *otr);

tstatic otrng_in_message_type get_message_type(const string_p message);

tstatic otrng_err extract_header(otrng_header_s *dst, const uint8_t *buffer,
                                 const size_t bufflen);

tstatic tlv_s *otrng_smp_initiate(const client_profile_s *initiator_profile,
                                  const client_profile_s *responder_profile,
                                  const uint8_t *question, const size_t q_len,
                                  const uint8_t *secret, const size_t secretlen,
                                  uint8_t *ssid, smp_context_p smp,
                                  otrng_conversation_state_s *conversation);

tstatic const client_profile_s *get_my_client_profile(otrng_s *otr);

tstatic tlv_s *process_tlv(const tlv_s *tlv, otrng_s *otr);

tstatic tlv_s *otrng_smp_provide_secret(otrng_smp_event_t *event,
                                        smp_context_p smp,
                                        const client_profile_s *our_profile,
                                        const client_profile_s *their_profile,
                                        uint8_t *ssid, const uint8_t *secret,
                                        const size_t secretlen);

#endif

#endif
