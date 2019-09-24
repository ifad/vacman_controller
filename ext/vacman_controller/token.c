/*
 * Vacman Controller wrapper
 *
 * This Ruby Extension wraps the VASCO Vacman Controller
 * library and makes its API accessible to Ruby code.
 *
 * (C) 2013 https://github.com/mlankenau
 * (C) 2019 m.barnaba@ifad.org
 */
#include "vacman_controller.h"

/*
 * Vacman properties names and IDs registry
 */
struct token_property {
  const char *name;
  aat_int32 id;
};
static struct token_property vacman_token_properties[] = {
  {"token_model",                   TOKEN_MODEL                   },
  {"token_status",                  TOKEN_STATUS                  },
  {"use_count",                     USE_COUNT                     },
  {"last_time_used",                LAST_TIME_USED                },
  {"last_time_shift",               LAST_TIME_SHIFT               },
  {"time_based_algo",               TIME_BASED_ALGO               },
  {"event_based_algo",              EVENT_BASED_ALGO              },
  {"pin_supported",                 PIN_SUPPORTED                 },
  {"unlock_supported",              UNLOCK_SUPPORTED              },
  {"pin_ch_on",                     PIN_CH_ON                     },
  {"pin_change_enabled",            PIN_CH_ON                     },
  {"pin_len",                       PIN_LEN                       },
  {"pin_length",                    PIN_LEN                       },
  {"pin_min_len",                   PIN_MIN_LEN                   },
  {"pin_minimum_length",            PIN_MIN_LEN                   },
  {"pin_enabled",                   PIN_ENABLED                   },
  {"pin_ch_forced",                 PIN_CH_FORCED                 },
  {"pin_change_forced",             PIN_CH_FORCED                 },
  {"virtual_token_type",            VIRTUAL_TOKEN_TYPE            },
  {"virtual_token_grace_period",    VIRTUAL_TOKEN_GRACE_PERIOD    },
  {"virtual_token_remain_use",      VIRTUAL_TOKEN_REMAIN_USE      },
  {"last_response_type",            LAST_RESPONSE_TYPE            },
  {"error_count",                   ERROR_COUNT                   },
  {"event_value",                   EVENT_VALUE                   },
  {"last_event_value",              LAST_EVENT_VALUE              },
  {"sync_windows",                  SYNC_WINDOWS                  },
  {"primary_token_enabled",         PRIMARY_TOKEN_ENABLED         },
  {"virtual_token_supported",       VIRTUAL_TOKEN_SUPPORTED       },
  {"virtual_token_enabled",         VIRTUAL_TOKEN_ENABLED         },
  {"code_word",                     CODE_WORD                     },
  {"auth_mode",                     AUTH_MODE                     },
  {"ocra_suite",                    OCRA_SUITE                    },
  {"derivation_supported",          DERIVATION_SUPPORTED          },
  {"max_dtf_number",                MAX_DTF_NUMBER                },
  {"response_len",                  RESPONSE_LEN                  },
  {"response_length",               RESPONSE_LEN                  },
  {"response_format",               RESPONSE_FORMAT               },
  {"response_chk",                  RESPONSE_CHK                  },
  {"response_checksum",             RESPONSE_CHK                  },
  {"time_step",                     TIME_STEP                     },
  {"use_3des",                      TRIPLE_DES_USED               },
  {"triple_des_used",               TRIPLE_DES_USED               },
};

static size_t vacman_token_properties_count = sizeof(vacman_token_properties)/sizeof(struct token_property);

/*
 * Convert property name to property ID
 */
static long vacman_token_get_property_id(char *property_name) {
  for (size_t i = 0; i < vacman_token_properties_count; i++) {
    if (strcmp(property_name, vacman_token_properties[i].name) == 0) {
      return vacman_token_properties[i].id;
    }
  }

  rb_raise(e_VacmanError, "Invalid property name `%s'", property_name);
  return 0;
}


/*
 * Get token property names
 */
VALUE vacman_token_get_property_names() {
  VALUE ret = rb_ary_new();

  for (size_t i = 0; i < vacman_token_properties_count; i++) {
    const char *name = vacman_token_properties[i].name;
    rb_ary_push(ret, rb_str_new2(name));
  }

  return ret;
}


/*
 * Get the given property value from the given token.
 */
VALUE vacman_token_get_property(VALUE module, VALUE token, VALUE property) {
  TDigipassBlob dpdata;
  vacman_rbhash_to_digipass(token, &dpdata);

  aat_ascii value[64];
  aat_int32 property_id = vacman_token_get_property_id(StringValueCStr(property));
  aat_int32 result = AAL2GetTokenProperty(&dpdata, &g_KernelParms, property_id, value);

  if (result == 0) {
    return rb_str_new2(value);
  } else {
    vacman_library_error("AAL2GetTokenProperty", result);
    return Qnil;
  }
}


/*
 * Set the given token property to the given value.
 */
VALUE vacman_token_set_property(VALUE module, VALUE token, VALUE property, VALUE rbval) {
  TDigipassBlob dpdata;

  aat_int32 property_id = vacman_token_get_property_id(StringValueCStr(property));
  aat_int32 value = rb_fix2int(rbval);

  vacman_rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2SetTokenProperty(&dpdata, &g_KernelParms, property_id, value);

  vacman_digipass_to_rbhash(&dpdata, token);

  if (result == 0) {
    return Qtrue;
  } else {
    vacman_library_error("AAL2SetTokenProperty", result);
    return Qnil;
  }
}


/*
 * Changes the static password on the given token.
 */
VALUE vacman_token_set_pin(VALUE module, VALUE token, VALUE pin) {
  TDigipassBlob dpdata;

  if (!RB_TYPE_P(pin, T_STRING)) {
    rb_raise(e_VacmanError, "invalid pin given, requires a string");
    return Qnil;
  }

  vacman_rbhash_to_digipass(token, &dpdata);

  aat_ascii *passwd = StringValueCStr(pin);
  aat_int32 result = AAL2ChangeStaticPassword(&dpdata, &g_KernelParms, passwd, passwd);

  vacman_digipass_to_rbhash(&dpdata, token);

  if (result == 0) {
    return Qtrue;
  } else {
    vacman_library_error("AAL2ChangeStaticPassword", result);
    return Qnil;
  }
}


/*
 * Resets the token error count and the time window shift.
 */
VALUE vacman_token_reset_info(VALUE module, VALUE token) {
  TDigipassBlob dpdata;

  vacman_rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2ResetTokenInfo(&dpdata, &g_KernelParms);

  vacman_digipass_to_rbhash(&dpdata, token);

  if (result == 0) {
    return Qtrue;
  } else {
    vacman_library_error("AAL2ResetTokenInfo", result);
    return Qnil;
  }
}


/*
 * Verifies the given OTP against the given token.
 */
VALUE vacman_token_verify_password(VALUE module, VALUE token, VALUE password) {
  TDigipassBlob dpdata;

  vacman_rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2VerifyPassword(&dpdata, &g_KernelParms, rb_string_value_cstr(&password), 0);

  vacman_digipass_to_rbhash(&dpdata, token);

  if (result == 0)
    return Qtrue;
  else {
    vacman_library_error("AAL2VerifyPassword", result);
    return Qnil;
  }
}

/*
 * Generate an OTP from the given token, if the token allows it.
 */
VALUE vacman_token_generate_password(VALUE module, VALUE token) {
  TDigipassBlob dpdata;

  vacman_rbhash_to_digipass(token, &dpdata);

  aat_ascii password[18];
  memset(password, 0, sizeof(password));

  aat_int32 result = AAL2GenPassword(&dpdata, &g_KernelParms, password, NULL);
  vacman_digipass_to_rbhash(&dpdata, token);

  if (result != 0) {
    vacman_library_error("AAL2GenPassword", result);
    return Qnil;
  }

  return rb_str_new2(password);
}
