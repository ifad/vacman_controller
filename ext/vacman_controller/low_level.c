/*
 * Vacman Controller wrapper
 *
 * This Ruby Extension wraps the VASCO Vacman Controller
 * library and makes its API accessible to Ruby code.
 *
 * (C) 2013 https://github.com/mlankenau
 * (C) 2019 m.barnaba@ifad.org
 */

#include <ruby.h>
#include <string.h>
#include <aal2sdk.h>


/* The Vacman default kernel parameters, set up upon extension initialisation. */
TKernelParms g_KernelParms;

/* Ruby exception type, defined as VacmanController::Error in Ruby land. */
static VALUE e_VacmanError;


/*
 * Raises an Error, decoding the Vacman Controller error code.
 */
static void vacman_library_error(const char* method, int vacman_error_code) {
  aat_ascii vacman_error_message[100]; // Recommended value in documentation.

  AAL2GetErrorMsg(vacman_error_code, vacman_error_message);

  char error_message[256];
  snprintf(error_message, 255, "%s error %d: %s", method, vacman_error_code,
           vacman_error_message);

  VALUE exc = rb_exc_new2(e_VacmanError, error_message);
  rb_iv_set(exc, "@library_method", rb_str_new2(method));
  rb_iv_set(exc, "@error_code",     INT2FIX(vacman_error_code));
  rb_iv_set(exc, "@error_message",  rb_str_new2(vacman_error_message));

  rb_exc_raise(exc);
}


/*
 * Use AAL2GetLibraryVersion to obtain library version and return it as a Ruby Hash
 */
static VALUE vacman_library_version(VALUE module) {
  aat_ascii version[16];
  aat_int32 version_len = sizeof(version);

  aat_ascii bitness[4];
  aat_int32 bitness_len = sizeof(bitness);

  aat_ascii type[8];
  aat_int32 type_len = sizeof(type);

  aat_int32 result = AAL2GetLibraryVersion(version, &version_len, bitness,
      &bitness_len, type, &type_len);

  if (result != 0) {
    vacman_library_error("AAL2GetLibraryVersion", result);
    return Qnil;
  }

  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, rb_str_new2("version"), rb_str_new2(version));
  rb_hash_aset(hash, rb_str_new2("bitness"), rb_str_new2(bitness));
  rb_hash_aset(hash, rb_str_new2("type"),    rb_str_new2(type));

  return hash;
}


/*
 * Convert a TDigipassBlob structure into a Ruby Hash
 */
static void digipass_to_rbhash(TDigipassBlob* dpdata, VALUE hash) {
  char buffer[256];

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->Serial, 10);
  rb_hash_aset(hash, rb_str_new2("serial"), rb_str_new2(buffer));

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->AppName, 12);
  rb_hash_aset(hash, rb_str_new2("app_name"), rb_str_new2(buffer));

  memset(buffer, 0, sizeof(buffer));
  strncpy(buffer, dpdata->Blob, 224);
  rb_hash_aset(hash, rb_str_new2("blob"), rb_str_new2(buffer));

  rb_hash_aset(hash, rb_str_new2("flags1"), rb_fix_new(dpdata->DPFlags[0]));
  rb_hash_aset(hash, rb_str_new2("flags2"), rb_fix_new(dpdata->DPFlags[1]));
}


/*
 * Gets the given property from the given token hash and raises an Error
 * if the following conditions occur:
 *
 * * The key is not found
 * * The key is not of the given type
 *
 * Otherwise, the value corresponding to the key is returned.
 *
 */
static VALUE rbhash_get_key(VALUE token, const char *property, int type) {
  VALUE ret = rb_hash_aref(token, rb_str_new2(property));

  if (ret == Qnil) {
    rb_raise(e_VacmanError, "invalid token object given: %s property is nil", property);
    return Qnil;
  }

  if (!RB_TYPE_P(ret, type)) {
    rb_raise(e_VacmanError, "invalid token object given: %s property is not of the correct type", property);
    return Qnil;
  }

  return ret;
}


/*
 * Convert a Ruby Hash with the required keys to a TDigipassBlob structure.
 */
static void rbhash_to_digipass(VALUE token, TDigipassBlob* dpdata) {
  if (!RB_TYPE_P(token, T_HASH)) {
    rb_raise(e_VacmanError, "invalid token object given, requires an hash");
    return;
  }

  VALUE blob     = rbhash_get_key(token, "blob",     T_STRING);
  VALUE serial   = rbhash_get_key(token, "serial",   T_STRING);
  VALUE app_name = rbhash_get_key(token, "app_name", T_STRING);
  VALUE flag1    = rbhash_get_key(token, "flags1",   T_FIXNUM);
  VALUE flag2    = rbhash_get_key(token, "flags2",   T_FIXNUM);

  memset(dpdata, 0, sizeof(*dpdata));

  strcpy(dpdata->Blob, rb_string_value_cstr(&blob));
  strncpy(dpdata->Serial, rb_string_value_cstr(&serial), sizeof(dpdata->Serial));
  strncpy(dpdata->AppName, rb_string_value_cstr(&app_name), sizeof(dpdata->AppName));
  dpdata->DPFlags[0] = rb_fix2int(flag1);
  dpdata->DPFlags[1] = rb_fix2int(flag2);
}



/*
 * Generate an OTP from the given token, if the token allows it.
 */
static VALUE vacman_generate_password(VALUE module, VALUE token) {
  TDigipassBlob dpdata;

  rbhash_to_digipass(token, &dpdata);

  aat_ascii password[18];
  memset(password, 0, sizeof(password));

  aat_int32 result = AAL2GenPassword(&dpdata, &g_KernelParms, password, NULL);
  digipass_to_rbhash(&dpdata, token);

  if (result != 0) {
    vacman_library_error("AAL2GenPassword", result);
    return Qnil;
  }

  return rb_str_new2(password);
}


/*
 * Vacman properties names and IDs registry
 */
struct token_property {
  const char *name;
  aat_int32 id;
};
static struct token_property token_properties[] = {
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

static size_t token_properties_count = sizeof(token_properties)/sizeof(struct token_property);

/*
 * Convert property name to property ID
 */
static long vacman_get_property_id(char *property_name) {
  for (size_t i = 0; i < token_properties_count; i++) {
    if (strcmp(property_name, token_properties[i].name) == 0) {
      return token_properties[i].id;
    }
  }

  rb_raise(e_VacmanError, "Invalid property name `%s'", property_name);
  return 0;
}


/*
 * Get token property names
 */
static VALUE vacman_get_token_property_names(void) {
  VALUE ret = rb_ary_new();

  for (size_t i = 0; i < token_properties_count; i++) {
    const char *name = token_properties[i].name;
    rb_ary_push(ret, rb_str_new2(name));
  }

  return ret;
}


/*
 * Get the given property value from the given token.
 */
static VALUE vacman_get_token_property(VALUE module, VALUE token, VALUE property) {
  TDigipassBlob dpdata;
  rbhash_to_digipass(token, &dpdata);

  aat_ascii value[64];
  aat_int32 property_id = vacman_get_property_id(StringValueCStr(property));
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
static VALUE vacman_set_token_property(VALUE module, VALUE token, VALUE property, VALUE rbval) {
  TDigipassBlob dpdata;

  aat_int32 property_id = vacman_get_property_id(StringValueCStr(property));
  aat_int32 value = rb_fix2int(rbval);

  rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2SetTokenProperty(&dpdata, &g_KernelParms, property_id, value);

  digipass_to_rbhash(&dpdata, token);

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
static VALUE vacman_set_token_pin(VALUE module, VALUE token, VALUE pin) {
  TDigipassBlob dpdata;

  if (!RB_TYPE_P(pin, T_STRING)) {
    rb_raise(e_VacmanError, "invalid pin given, requires a string");
    return Qnil;
  }

  rbhash_to_digipass(token, &dpdata);

  aat_ascii *passwd = StringValueCStr(pin);
  aat_int32 result = AAL2ChangeStaticPassword(&dpdata, &g_KernelParms, passwd, passwd);

  digipass_to_rbhash(&dpdata, token);

  if (result == 0) {
    return Qtrue;
  } else {
    vacman_library_error("AAL2ChangeStaticPassword", result);
    return Qnil;
  }
}


/*
 * Verifies the given OTP against the given token.
 */
static VALUE vacman_verify_password(VALUE module, VALUE token, VALUE password) {
  TDigipassBlob dpdata;

  rbhash_to_digipass(token, &dpdata);

  aat_int32 result = AAL2VerifyPassword(&dpdata, &g_KernelParms, rb_string_value_cstr(&password), 0);

  digipass_to_rbhash(&dpdata, token);

  if (result == 0)
    return Qtrue;
  else {
    vacman_library_error("AAL2VerifyPassword", result);
    return Qnil;
  }
}


/*
 * Imports a .DPX file containing token seeds and initialisation values.
 *
 * Pass the pre-shared key to validate it as the second argument. The
 * key is not validated by the AAL2 library, if you pass a different
 * key than the one that was used to create the DPX, you will get back
 * tokens that generate different OTPs.
 *
 */
static VALUE vacman_import(VALUE module, VALUE filename, VALUE key) {
  TDPXHandle dpx_handle;
  aat_int16 appl_count;
  aat_ascii appl_names[13*8];
  aat_int16 token_count;

  aat_int32 result = AAL2DPXInit(&dpx_handle,
                                 rb_string_value_cstr(&filename),
                                 rb_string_value_cstr(&key),
                                 &appl_count,
                                 appl_names,
                                 &token_count);

  if (result != 0) {
    vacman_library_error("AAL2DPXInit", result);
    return Qnil;
  }

  aat_ascii sw_out_serial_No[22+1];
  aat_ascii sw_out_type[5+1];
  aat_ascii sw_out_authmode[2+1];
  TDigipassBlob dpdata;

  VALUE list = rb_ary_new();

  while (1) {
    result = AAL2DPXGetToken(&dpx_handle,
        &g_KernelParms,
        appl_names,
        sw_out_serial_No,
        sw_out_type,
        sw_out_authmode,
        &dpdata);


    if (result < 0) {
      vacman_library_error("AAL2DPXGetToken", result);
      return Qnil;
    }

    if (result == 107) break;

    VALUE hash = rb_hash_new();

    digipass_to_rbhash(&dpdata, hash);

    rb_ary_push(list, hash);
  }

  AAL2DPXClose(&dpx_handle);

  return list;
}


/*
 * Vacman Controller kernel properties
 */
struct kernel_property {
  const char *name;
  aat_int32 *value;
  aat_int32 deflt;
};
static struct kernel_property kernel_properties[] = {
  { "ITimeWindow",    &g_KernelParms.ITimeWindow,    30  },  // Identification Window size in time steps
  { "STimeWindow",    &g_KernelParms.STimeWindow,    24  },  // Signature Window size in secs
  { "DiagLevel",      &g_KernelParms.DiagLevel,      0   },  // Requested Diagnostic Level
  { "GMTAdjust",      &g_KernelParms.GMTAdjust,      0   },  // GMT Time adjustment to perform
  { "CheckChallenge", &g_KernelParms.CheckChallenge, 0   },  // Verify Challenge Corrupted (mandatory for Gordian)
  { "IThreshold",     &g_KernelParms.IThreshold,     3   },  // Identification Error Threshold
  { "SThreshold",     &g_KernelParms.SThreshold,     1   },  // Signature Error Threshold
  { "ChkInactDays",   &g_KernelParms.ChkInactDays,   0   },  // Check Inactive Days
  { "DeriveVector",   &g_KernelParms.DeriveVector,   0   },  // Vector used to make Data Encryption unique
  { "SyncWindow",     &g_KernelParms.SyncWindow,     2   },  // Synchronisation Time Window (h)
  { "OnLineSG",       &g_KernelParms.OnLineSG,       2   },  // On line signature
  { "EventWindow",    &g_KernelParms.EventWindow,    100 },  // Event Window size in nbr of iterations
  { "HSMSlotId",      &g_KernelParms.HSMSlotId,      0   },  // HSM Slot id uses to store DB and Transport Key
};
static size_t kernel_properties_count = sizeof(kernel_properties)/sizeof(struct kernel_property);

/*
 * Initialise the kernel parameters with their defaults
 */
static void vacman_init_kernel_params() {
  memset(&g_KernelParms, 0, sizeof(g_KernelParms));

  g_KernelParms.ParmCount = 19; /* Number of valid parameters in this list */

  for (size_t i = 0; i < kernel_properties_count; i++) {
    *kernel_properties[i].value = kernel_properties[i].deflt;
  }
}


/*
 * Get kernel parameter names
 */
static VALUE vacman_get_kernel_property_names(void) {
  VALUE ret = rb_ary_new();

  for (size_t i = 0; i < kernel_properties_count; i++) {
    const char *name = kernel_properties[i].name;
    rb_ary_push(ret, rb_str_new2(name));
  }

  return ret;
}


/*
 * Set kernel parameter
 */
static VALUE vacman_set_kernel_param(VALUE module, VALUE paramname, VALUE rbval) {
  char *name = StringValueCStr(paramname);
  int value  = rb_fix2int(rbval);

  for (size_t i = 0; i < kernel_properties_count; i++) {
    if (strcmp(name, kernel_properties[i].name) == 0) {
      *kernel_properties[i].value = value;
      return Qtrue;
    }
  }

  rb_raise(e_VacmanError, "Invalid kernel param %s", name);
  return Qnil;
}


/*
 * Get kernel parameter
 */
static VALUE vacman_get_kernel_param(VALUE module, VALUE paramname) {
  char *name = StringValueCStr(paramname);

  for (size_t i = 0; i < kernel_properties_count; i++) {
    if (strcmp(name, kernel_properties[i].name) == 0) {
      return LONG2FIX(*kernel_properties[i].value);
    }
  }

  rb_raise(e_VacmanError, "Invalid kernel param %s", name);
  return Qnil;
}


/*
 * Extension entry point
 */
void Init_low_level(void) {
  VALUE controller = rb_define_module("VacmanController");
  VALUE lowlevel   = rb_define_module_under(controller, "LowLevel");

  e_VacmanError = rb_define_class_under(controller, "Error", rb_eStandardError);

  vacman_init_kernel_params();

  /* Global methods */
  rb_define_singleton_method(lowlevel, "library_version",       vacman_library_version, 0);
  rb_define_singleton_method(lowlevel, "import",                vacman_import, 2);

  /* Token methods */
  rb_define_singleton_method(lowlevel, "token_property_names",  vacman_get_token_property_names, 0);
  rb_define_singleton_method(lowlevel, "get_token_property",    vacman_get_token_property, 2);
  rb_define_singleton_method(lowlevel, "set_token_property",    vacman_set_token_property, 3);
  rb_define_singleton_method(lowlevel, "generate_password",     vacman_generate_password, 1);
  rb_define_singleton_method(lowlevel, "verify_password",       vacman_verify_password, 2);
  rb_define_singleton_method(lowlevel, "set_token_pin",         vacman_set_token_pin, 2);

  /* Kernel methods */
  rb_define_singleton_method(lowlevel, "kernel_property_names", vacman_get_kernel_property_names, 0);
  rb_define_singleton_method(lowlevel, "get_kernel_param",      vacman_get_kernel_param, 1);
  rb_define_singleton_method(lowlevel, "set_kernel_param",      vacman_set_kernel_param, 2);
}
