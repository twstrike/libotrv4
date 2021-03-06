/*
 *  This file is part of the Off-the-Record Next Generation Messaging
 *  library (libotr-ng).
 *
 *  Copyright (C) 2016-2019, the libotr-ng contributors.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
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

#include <glib.h>
#include <string.h>

#include "test_helpers.h"

#include "test_fixtures.h"

#include "instance_tag.h"

static void test_instance_tag_generates_tag_when_file_empty() {
  const char *alice_coy_account = "alice@coy.im";
  const char *xmpp_protocol = "XMPP";

  FILE *tmpFILEp = tmpfile();

  otrng_instag_s *instag = otrng_xmalloc_z(sizeof(otrng_instag_s));
  otrng_assert(otrng_instag_get(instag, alice_coy_account, xmpp_protocol,
                                tmpFILEp) == otrng_true);

  fclose(tmpFILEp);

  g_assert_cmpint(instag->value, !=, 0);
  g_assert_cmpint(instag->value, >, OTRNG_MIN_VALID_INSTAG);

  g_assert_cmpstr(instag->account, ==, alice_coy_account);
  g_assert_cmpstr(instag->protocol, ==, xmpp_protocol);

  otrng_instag_free(instag);
}

static int INSTAG_CB_CALLED = 0;
static void test_create_instag_cb(otrng_client_s *client) {
  (void)client;
  INSTAG_CB_CALLED = 1;
}

static void test_invokes_create_instag_callbacks(void) {
  otrng_client_callbacks_s callbacks = {.create_instag =
                                            &test_create_instag_cb};

  otrng_client_callbacks_create_instag(&callbacks, NULL);
  otrng_assert(INSTAG_CB_CALLED);
}

static void test_instance_tag_generates_tag_when_file_is_full() {
  const char *icq_alice_account = "alice_icq";
  const char *icq_protocol = "ICQ";
  const char *xmpp_alice_account = "alice_xmpp";
  const char *xmpp_protocol = "XMPP";
  const char *irc_alice_account = "alice_irc";
  const char *irc_protocol = "IRC";
  unsigned int icq_instag_value = 0x9abcdef0;

  FILE *tmpFILEp = tmpfile();

  fprintf(tmpFILEp, "%s\t%s\t%08x\n", icq_alice_account, icq_protocol,
          icq_instag_value);

  rewind(tmpFILEp);

  otrng_instag_s *first_instag = otrng_xmalloc_z(sizeof(otrng_instag_s));
  otrng_assert(otrng_instag_get(first_instag, icq_alice_account, icq_protocol,
                                tmpFILEp) == otrng_true);

  otrng_instag_s *second_instag = otrng_xmalloc_z(sizeof(otrng_instag_s));
  otrng_assert(otrng_instag_get(second_instag, xmpp_alice_account,
                                xmpp_protocol, tmpFILEp) == otrng_true);

  otrng_instag_s *third_instag = otrng_xmalloc_z(sizeof(otrng_instag_s));
  otrng_assert(otrng_instag_get(third_instag, irc_alice_account, irc_protocol,
                                tmpFILEp) == otrng_true);

  fclose(tmpFILEp);

  char sone[9];
  snprintf(sone, sizeof(sone), "%08x", first_instag->value);

  g_assert_cmpstr(first_instag->account, ==, icq_alice_account);
  g_assert_cmpstr(first_instag->protocol, ==, icq_protocol);
  g_assert_cmpint(first_instag->value, !=, 0);
  g_assert_cmpint(first_instag->value, >, OTRNG_MIN_VALID_INSTAG);
  g_assert_cmpstr(sone, ==, "9abcdef0");

  g_assert_cmpstr(second_instag->account, ==, xmpp_alice_account);
  g_assert_cmpstr(second_instag->protocol, ==, xmpp_protocol);
  g_assert_cmpint(second_instag->value, !=, 0);
  g_assert_cmpint(second_instag->value, >, OTRNG_MIN_VALID_INSTAG);

  g_assert_cmpstr(third_instag->account, ==, irc_alice_account);
  g_assert_cmpstr(third_instag->protocol, ==, irc_protocol);
  g_assert_cmpint(third_instag->value, !=, 0);
  g_assert_cmpint(third_instag->value, >, OTRNG_MIN_VALID_INSTAG);

  otrng_instag_free(first_instag);
  otrng_instag_free(second_instag);
  otrng_instag_free(third_instag);
}

void units_instance_tag_add_tests(void) {
  g_test_add_func("/otrng/instance_tag/generates_when_file_empty",
                  test_instance_tag_generates_tag_when_file_empty);
  g_test_add_func("/otrng/instance_tag/generates_when_file_is_full",
                  test_instance_tag_generates_tag_when_file_is_full);
  g_test_add_func("/otrng/instance_tag/otrng_invokes_create_instag",
                  test_invokes_create_instag_callbacks);
}
